#ifndef TRANSITCALCULATOR_H
#define TRANSITCALCULATOR_H

#include <QObject>
#include <QVector>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>

struct Stop{
    Stop(const unsigned int num, const long kx, const long ky, const double klat, const double klon):
        number{num}, x{kx}, y{ky}, latitude{klat}, longitude{klon} {}
    Stop(): number{0}, x{0}, y{0}, latitude{0.0}, longitude{0.0} {}
    Stop& operator =(const Stop& lhs)
    {
        number = lhs.number;
        x = lhs.x;
        y = lhs.y;
        latitude = lhs.latitude;
        longitude = lhs.longitude;
        return *this;
    }
    unsigned int number;
    long x;
    long y;
    double latitude;
    double longitude;
};

struct Variant {
    Variant(const QString valKey) : key{valKey}{}
    QString key;
    QVector<Stop> stops;
};

struct Route {
    Route(const QString valKey): key{valKey} {}
    QString key;
    QVector<Variant> variants;

};



class TransitCalculator : public QObject
{
    Q_OBJECT
public:
    explicit TransitCalculator(QObject *parent = nullptr);
    void computeDistances();

signals:

private:
    void extractStopTransitData(const QJsonObject stopObj, Stop& stop);
    double toRad(double degree);
    double calculateDistance(double lat1, double long1, double lat2, double long2);
    void sendRequest(const QString strUrl, QString& strReply);

private:
    QVector<Route> routes;
    double maxDistance;
    QString routeName;
    QString variantName;


};

#endif // TRANSITCALCULATOR_H
