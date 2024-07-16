#include "transitcalculator.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QEventLoop>
#include <math.h>
#include <iostream>

TransitCalculator::TransitCalculator(QObject *parent)
    : QObject{parent}
{

    maxDistance = 0.0;
    routeName = "";
    variantName = "";
}

void TransitCalculator::extractStopTransitData(const QJsonObject stopObj, Stop& stop)
{
    //QString strNumber =  stopObj["number"].toString();
    stop.number =  stopObj["number"].toInt();
    QJsonObject centerObj = stopObj.value("centre").toObject();
    QJsonObject utmObj = centerObj.value("utm").toObject();
    stop.x = utmObj["x"].toInt();
    stop.y = utmObj["y"].toInt();
    QJsonObject jsonGeographic = centerObj.value("geographic").toObject();

    stop.latitude = jsonGeographic["latitude"].toString().toDouble();
    stop.longitude = jsonGeographic["longitude"].toString().toDouble();
}

double TransitCalculator::toRad(double degree) {
    return degree/180 * M_PI;
}

double TransitCalculator::calculateDistance(double lat1, double long1, double lat2, double long2) {
    double dist = sin(toRad(lat1)) * sin(toRad(lat2)) + cos(toRad(lat1)) * cos(toRad(lat2)) * cos(toRad(long1 - long2));
    dist = acos(dist);
    return (6371 * dist);
}

void TransitCalculator::sendRequest(const QString strUrl, QString& strReply)
{
    QEventLoop eventLoop;
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    QNetworkRequest req{QUrl(strUrl)};
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {

        strReply = (QString)reply->readAll();
        //qDebug() << "Response:" << strReply;
        delete reply;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
}

void TransitCalculator::computeDistances()
{
    QStringList routelist = (QStringList() << "16" << "18" << "33" << "60" << "BLUE");

    for (const QString & strRoute : routelist) {

        QString urlroute = QString("https://api.winnipegtransit.com/v3/routes/%1.json?api-key=o10vC-aKuT_1jvUWgQEK").arg(strRoute);
        QString strReplyRoute("");
        sendRequest(urlroute, strReplyRoute);
        //QJsonDocument jsonDocRouteData = QJsonDocument::fromJson(networkHelper->data);
        QJsonDocument jsonDocRouteData = QJsonDocument::fromJson(strReplyRoute.toUtf8());
        QJsonObject jsonRouteEncloseObj = jsonDocRouteData.object();
        QJsonObject routeObj = jsonRouteEncloseObj.value("route").toObject();

        QJsonArray jsonArrayVariants = routeObj.value("variants").toArray();
        QString routeKey = routeObj.value("key").toString();
        //initialize a route
        Route route(routeKey);

        //QJsonArray jsonArrayVariants; // = routeObj.value("variants").toArray();
        //qDebug() << "Array:" << jsonArrayVariants;

        foreach (const QJsonValue & value, jsonArrayVariants) {
            QJsonObject variantObj = value.toObject();
            QString variantKey =  variantObj["key"].toString();
            //qDebug() << "Variant Key:" << variantKey;
            Variant variant(variantKey);

            QString urlvariant = QString("https://api.winnipegtransit.com/v3/stops.json?variant=%1&api-key=o10vC-aKuT_1jvUWgQEK").arg(variantKey);
            QString strReplyVariant("");
            sendRequest(urlvariant, strReplyVariant);
            QJsonDocument jsonVariantData = QJsonDocument::fromJson(strReplyVariant.toUtf8());
            QJsonObject jsonVariantObj = jsonVariantData.object();
            //qDebug() << "Variant Object:" << jsonVariantObj;

            QJsonArray jsonStops = jsonVariantObj.value("stops").toArray();
            //qDebug() << "Stops:" << jsonStops;
            QJsonValue valStop = jsonStops.at(0);
            QJsonObject objStop = valStop.toObject();
            Stop prevStop;
            extractStopTransitData(objStop, prevStop);
            variant.stops.push_back(prevStop);
            double variantDistance = 0.0;
            for (unsigned int i = 1; i < jsonStops.size(); ++i) {
                QJsonValue valStop = jsonStops.at(i);
                QJsonObject objStop = valStop.toObject();
                Stop curStop;
                extractStopTransitData(objStop, curStop);

                variantDistance += calculateDistance(prevStop.latitude,  prevStop.longitude,  curStop.latitude,  curStop.longitude);
                prevStop = curStop;
                variant.stops.push_back(prevStop);
            }
            if (variantDistance > maxDistance) {
                maxDistance = variantDistance;
                routeName = strRoute;
                variantName = variantKey;
            }
            route.variants.push_back(variant);
        }
    }
    qDebug() << "Max Distance: " << maxDistance << "Route: "<< routeName << "Variant: " << variantName;
    std::cout << "Max Distance: " << maxDistance << std::endl;
    std::cout << "Route: " << routeName.toStdString() << std::endl;
    std::cout << "Variant: " << variantName.toStdString() << std::endl;

}



