#include <BALL/PYTHON/pyServer.h>

#include <BALL/COMMON/logStream.h>
#include <BALL/PYTHON/pyInterpreter.h>

#include <QtCore/QDataStream>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QTcpSocket>

using std::string;
using std::tie;

namespace BALL {
	PyServer::PyServer()
	{
		if (!PyInterpreter::isInitialized())
		{
			Log.error() << "[PyServer] Server cannot be started as the Python interpreter is unavailable!" << std::endl;
			return;
		}
		server_.reset(new QTcpServer());
		server_->listen(QHostAddress::LocalHost, 8897u);
		server_->connect(server_.get(), &QTcpServer::newConnection, [this]{ this->processRequest(); });
	}

	void PyServer::processRequest()
	{
		auto client = server_->nextPendingConnection();

		if (!client) return;

		client->waitForReadyRead(10000);
		auto json = QJsonDocument::fromJson(client->readAll());

		if (json.isNull() || !json.isObject())
		{
			client->write(createMessage("error", "[PyServer] ERROR: Invalid request"));
			disconnectClient(client);
			return;
		}

		auto request  = json.object();
		auto msg_type = request.take("msg_type");
		auto content  = request.take("content");

		if (  msg_type == QJsonValue::Undefined || !msg_type.isString()
			|| content == QJsonValue::Undefined || !content.isString())
		{
			client->write(createMessage("error", "[PyServer] ERROR: Invalid request"));
			disconnectClient(client);
			return;
		}

		// delegate request
		if (msg_type.toString() == "execute_request")
		{
			processExecuteRequest(client, content.toString());
		}
		else
		{
			client->write(createMessage("error", "[PyServer] ERROR: Invalid request type"));
		}
		disconnectClient(client);
	}

	void PyServer::processExecuteRequest(QTcpSocket* client, const QString& content)
	{
		bool ok;
		string res;
		tie(ok, res) = PyInterpreter::run(content.toStdString());

		if (!ok)
		{
			client->write(createMessage("error", PyInterpreter::getErrorMessage().c_str()));
			return;
		}

		client->write(createMessage("execute_result", res.c_str()));
	}

	QByteArray PyServer::createMessage(QString msg_type, QString content)
	{
		QJsonObject msg;
		msg["msg_type"] = msg_type;
		msg["content"]  = content;

		if (msg["content"] == QJsonValue::Null)
		{
			msg["msg_type"] = "error";
			msg["content"] = "[PyServer] ERROR: Output could not be generated; It was probably too large.";
		}

		return QJsonDocument(msg).toJson(QJsonDocument::Compact);
	}

	void PyServer::disconnectClient(QTcpSocket* client)
	{
		client->waitForBytesWritten(10000);
		client->disconnectFromHost();
		client->deleteLater();
	}
}
