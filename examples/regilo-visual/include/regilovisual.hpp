/*
 * Regilo
 * Copyright (C) 2015-2016  Branislav Holý <branoholy@gmail.com>
 *
 * This file is part of Regilo.
 *
 * Regilo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Regilo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Regilo.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef REGILOVISUAL_HPP
#define REGILOVISUAL_HPP

#include <condition_variable>
#include <mutex>
#include <thread>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <regilo/controller.hpp>

class wxGCDC;

class RegiloVisual : public wxApp
{
private:
	std::mutex controllerMutex;
	regilo::Controller *controller;
	bool useScanner;
	bool manualScanning;
	bool moveScanning;

	wxFrame *frame;
	wxPanel *panel;
	regilo::ScanData data;

	std::thread scanThread;
	bool scanThreadRunning;
	std::condition_variable scanThreadCV;
	std::mutex scanThreadCVMutex;

	wxColour radarColor, pointColor;

	double radarAngle;
	double radarLength;
	std::thread radarThread;
	std::condition_variable radarThreadCV;
	std::mutex radarThreadCVMutex;

	wxImage radarGradient;

	void stopScanThread();
	void scanAndShow();

	wxRect getRotatedBoundingBox(const wxRect& rect, double angle);
	void drawRadarGradient(wxGCDC& gcdc, int width2, int height2);

public:
	RegiloVisual(regilo::Controller *controller, bool useScanner = true, bool manualScanning = false, bool moveScanning = false);

	virtual bool OnInit();
	virtual int OnExit();

	void setMotorByKey(wxKeyEvent& keyEvent);
	void repaint(wxPaintEvent& paintEvent);

	static void Display(wxApp *app, int& argc, char **argv);
};

#endif // REGILOVISUAL_HPP
