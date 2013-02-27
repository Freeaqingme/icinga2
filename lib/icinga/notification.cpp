/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012 Icinga Development Team (http://www.icinga.org/)        *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "i2-icinga.h"

using namespace icinga;

REGISTER_TYPE(Notification, NULL);

Notification::Notification(const Dictionary::Ptr& properties)
	: DynamicObject(properties)
{
	RegisterAttribute("notification_command", Attribute_Config, &m_NotificationCommand);
	RegisterAttribute("macros", Attribute_Config, &m_Macros);
	RegisterAttribute("users", Attribute_Config, &m_Users);
	RegisterAttribute("host_name", Attribute_Config, &m_HostName);
	RegisterAttribute("service", Attribute_Config, &m_Service);

	Service::RefreshNotificationsCache();
}

Notification::~Notification(void)
{
	Service::RefreshNotificationsCache();
}

bool Notification::Exists(const String& name)
{
	return (DynamicObject::GetObject("Notification", name));
}

Notification::Ptr Notification::GetByName(const String& name)
{
	DynamicObject::Ptr configObject = DynamicObject::GetObject("Notification", name);

	if (!configObject)
		BOOST_THROW_EXCEPTION(invalid_argument("Notification '" + name + "' does not exist."));

	return dynamic_pointer_cast<Notification>(configObject);
}

Service::Ptr Notification::GetService(void) const
{
	Host::Ptr host = Host::GetByName(m_HostName);

	if (m_Service.IsEmpty())
		return Host::GetHostCheckService(host);
	else
		return Host::GetServiceByShortName(host, m_Service);
}

Value Notification::GetNotificationCommand(void) const
{
	return m_NotificationCommand;
}

Dictionary::Ptr Notification::GetMacros(void) const
{
	return m_Macros;
}

set<User::Ptr> Notification::GetUsers(void) const
{
	set<User::Ptr> result;

	Dictionary::Ptr users = m_Users;

	if (users) {
		String name;
		BOOST_FOREACH(tie(tuples::ignore, name), users) {
			result.insert(User::GetByName(name));
		}
	}

	return result;
}

String Notification::NotificationTypeToString(NotificationType type)
{
	switch (type) {
		case NotificationDowntimeStart:
			return "DOWNTIMESTART";
		case NotificationDowntimeEnd:
			return "DOWNTIMEEND";
		case NotificationDowntimeRemoved:
			return "DOWNTIMECANCELLED";
		case NotificationCustom:
			return "CUSTOM";
		case NotificationAcknowledgement:
			return "ACKNOWLEDGEMENT";
		case NotificationProblem:
			return "PROBLEM";
		case NotificationRecovery:
			return "RECOVERY";
		default:
			return "UNKNOWN_NOTIFICATION";
	}
}

void Notification::BeginExecuteNotification(const Notification::Ptr& self, NotificationType type)
{

	vector<Dictionary::Ptr> macroDicts;

	Dictionary::Ptr notificationMacros = boost::make_shared<Dictionary>();
	notificationMacros->Set("NOTIFICATIONTYPE", NotificationTypeToString(type));
	macroDicts.push_back(notificationMacros);

	Service::Ptr service;
	set<User::Ptr> users;

	{
		ObjectLock olock(self);
		macroDicts.push_back(self->GetMacros());
		service = self->GetService();
		users = self->GetUsers();
	}

	Host::Ptr host;
	String service_name;

	{
		ObjectLock olock(service);
		macroDicts.push_back(service->GetMacros());
		service_name = service->GetName();
		host = service->GetHost();
	}

	macroDicts.push_back(Service::CalculateDynamicMacros(service));

	{
		ObjectLock olock(host);
		macroDicts.push_back(host->GetMacros());
		macroDicts.push_back(Host::CalculateDynamicMacros(host));
	}

	IcingaApplication::Ptr app = IcingaApplication::GetInstance();

	{
		ObjectLock olock(app);
		macroDicts.push_back(app->GetMacros());
	}

	macroDicts.push_back(IcingaApplication::CalculateDynamicMacros(app));

	Dictionary::Ptr macros = MacroProcessor::MergeMacroDicts(macroDicts);

	int count = 0;

	BOOST_FOREACH(const User::Ptr& user, users) {
		BeginExecuteNotificationHelper(self, macros, type, user);
		count++;
	}

	if (count == 0) {
		/* Send a notification even if there are no users specified. */
		BeginExecuteNotificationHelper(self, macros, type, User::Ptr());
	}
}

void Notification::BeginExecuteNotificationHelper(const Notification::Ptr& self, const Dictionary::Ptr& notificationMacros, NotificationType type, const User::Ptr& user)
{
	vector<Dictionary::Ptr> macroDicts;

	if (user) {
		{
			ObjectLock olock(self);
			macroDicts.push_back(user->GetMacros());
		}

		macroDicts.push_back(User::CalculateDynamicMacros(user));
	}

	macroDicts.push_back(notificationMacros);

	Dictionary::Ptr macros = MacroProcessor::MergeMacroDicts(macroDicts);

	ScriptTask::Ptr task;

	{
		ObjectLock olock(self);

		vector<Value> arguments;
		arguments.push_back(self);
		arguments.push_back(macros);
		arguments.push_back(type);
		task = self->MakeMethodTask("notify", arguments);

		if (!task) {
			Logger::Write(LogWarning, "icinga", "Notification object '" + self->GetName() + "' doesn't have a 'notify' method.");

			return;
		}

		/* We need to keep the task object alive until the completion handler is called. */
		self->m_Tasks.insert(task);
	}

	task->Start(boost::bind(&Notification::NotificationCompletedHandler, self, _1));
}

void Notification::NotificationCompletedHandler(const ScriptTask::Ptr& task)
{
	m_Tasks.erase(task);

	try {
		{
			ObjectLock tlock(task);
			(void) task->GetResult();
		}

		Logger::Write(LogInformation, "icinga", "Completed sending notification for service '" + GetService()->GetName() + "'");
	} catch (const exception& ex) {
		stringstream msgbuf;
		msgbuf << "Exception occured during notification for service '"
		       << GetService()->GetName() << "': " << diagnostic_information(ex);
		String message = msgbuf.str();

		Logger::Write(LogWarning, "icinga", message);
	}
}

void Notification::OnAttributeChanged(const String& name, const Value& oldValue)
{
	if (name == "host_name" || name == "service")
		Service::RefreshNotificationsCache();
}
