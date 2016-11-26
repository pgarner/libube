/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2016
 */

#include <QApplication>
#include <QMainWindow>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>

#include "lube/qwt.h"

namespace libube
{
    // Statics so the QApplication call can pass references
    static int sArgc = 0;
    static char** sArgv = 0;

    class MQwt : public qwt
    {
    public:
        MQwt();
        void exec() { mApp.exec(); };
        void plot();
        void curve(var iX, var iY, var iTitle);
        void axes(var iXLabel, var iYLabel);
    private:
        QApplication mApp;
        QMainWindow mMain;
        QwtPlot *mPlot;
    };

    void factory(Module** oModule, var iArg)
    {
        *oModule = new MQwt;
    }
}

using namespace libube;

MQwt::MQwt()
    : mApp(sArgc, sArgv)
{
    // 16:9
    mMain.resize(800, 450);
}

void MQwt::curve(var iX, var iY, var iTitle)
{
    QwtPlotCurve* curve = new QwtPlotCurve(iTitle.str());
    curve->attach(mPlot);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setSamples(
        iX.ptr<double>(), iY.ptr<double>(), std::min(iX.size(), iY.size())
    );
}

void MQwt::axes(var iXLabel, var iYLabel)
{
    mPlot->setAxisTitle(QwtPlot::xBottom, iXLabel.str());
    mPlot->setAxisTitle(QwtPlot::yLeft, iYLabel.str());
}

void MQwt::plot()
{
    mPlot = new QwtPlot(&mMain);
    mPlot->setCanvasBackground(QColor("White"));
    mPlot->setAutoReplot(true);

    // Grid; optional but I like them
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->attach(mPlot);
    grid->setMajorPen(Qt::black, 0, Qt::DotLine);
    grid->setMinorPen(Qt::gray, 0, Qt::DotLine);

    mMain.setCentralWidget(mPlot);
    mMain.show();
}
