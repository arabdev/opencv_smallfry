#include "opencv2/opencv.hpp"

#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace cv;
using namespace cv::ml;


struct GravitationalClustering
{
    struct Planet
    {
        Mat position;
        double mass;
        double radius;
        double ratio;
        int cls;

        Planet(const Mat &position=Mat(), double mass=0, double radius=0, int classb=0)
            : position(position)
            , mass(mass)
            , radius(radius)
            , cls(classb)
            , ratio(radius/mass)
        {}

        void addPoint(const Mat &pos, double m)
        {
            double summedMass = mass + m;
            double leftVal = mass / summedMass;
            double rightVal = m / summedMass;
            position = position*leftVal + pos*rightVal;
            radius = ratio * summedMass;
            mass = summedMass;
        }
    };

    double radius,mass;
    vector<Planet> planets;

    typedef std::pair<int,double> PlanetItem;

     GravitationalClustering(double radius=13.0, double mass=0.5)
        : radius(radius)
        , mass(mass)
    {}


    inline double relativePDF(double r,double sig) const
    {
        return exp(-pow(r, 2) / (2 * sig * sig));
    }
    inline double normalizedPDF(double r,double sig) const
    {
        const static double invsqrt2 = 1.0 / (2 * CV_PI);
        return invsqrt2 * relativePDF(r,sig) / sig;
    }

    vector<PlanetItem> findPlanetsInRadius(const Mat &pos, int cls) const
    {
        vector<PlanetItem> inrad;
        for (size_t i=0; i<planets.size(); i++)
        {
            if (cls != planets[i].cls)
                continue;

            double cur = norm(pos, planets[i].position, NORM_L2SQR);
            if (planets[i].radius >= cur)
            {
                inrad.push_back(make_pair(i, cur));
            }
        }
        return inrad;
    }
    Mat getDirectionalForceVector(const Mat &pos)
    {
        Mat directionalForceVector = Mat::zeros(pos.size(), pos.type());

        for (size_t i=0; i<planets.size(); i++)
        {
            double dist = norm(pos, planets[i].position);
            if (dist == 0)
            {
                return planets[i].position - pos;
            }
            Mat subt = planets[i].position - pos;
            double mag = (planets[i].mass / (dist * dist));
            directionalForceVector += subt * mag;
        }
        return directionalForceVector;
    }

    void onlineTrainSingular(const Mat &pos, int cls)
    {
        vector<PlanetItem> nearPlanets = findPlanetsInRadius(pos, cls);

        if (nearPlanets.empty())
        {
            planets.push_back(Planet(pos, mass, mass * radius, cls));
            return;
        }
        for (size_t i=0; i<nearPlanets.size(); i++)
        {
            PlanetItem p = nearPlanets[i];
            p.second = planets[p.first].mass / (p.second*p.second);
        }
        struct Sorter
        {
            bool operator()(const PlanetItem &a, const PlanetItem &b) const
            {
                return a.second > b.second;
            }
        };
        std::sort(nearPlanets.begin(), nearPlanets.end(), Sorter());

        planets[nearPlanets.front().first].addPoint(pos, mass);
    }

    int predictProbabilistic(const Mat &pos) const
    {
        map<int, pair<double,double> > pdf;
        for (size_t i=0; i<planets.size(); i++)
        {
            const Planet &p = planets[i];
            if (pdf.find(p.cls) == pdf.end())
                pdf[p.cls].first = 1.0;
            pdf[p.cls].second ++;
            pdf[p.cls].first *= relativePDF(norm(p.position, pos) / p.mass, p.radius*p.radius);
        }
        double maxm = -9999999.9;
        int maxid = -1;
        map<int, pair<double,double> >::iterator it = pdf.begin();
        for (; it != pdf.end(); it++)
        {
            double m = log(it->second.first) / it->second.second;
            if (m > maxm)
            {
                maxid = it->first;
                maxm = m;
            }
        }
        return maxid;
    }
};

