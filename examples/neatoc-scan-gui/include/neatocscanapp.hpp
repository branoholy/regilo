/*
 * NeatoC
 * Copyright (C) 2015-2016  Branislav Holý <branoholy@gmail.com>
 *
 * This file is part of NeatoC.
 *
 * NeatoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NeatoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NeatoC.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NEATOCSCANAPP_HPP
#define NEATOCSCANAPP_HPP

#include <condition_variable>
#include <mutex>
#include <thread>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include "neatoc/controller.hpp"

class NeatocScanApp : public wxApp
{
private:
	std::mutex controllerMutex;
	neatoc::Controller *controller;
	bool useScanner;
	bool manualScanning;
	bool moveScanning;

	wxFrame *frame;
	wxPanel *panel;
	neatoc::ScanData data;

	std::thread scanThread;
	bool scanThreadRunning;
	std::condition_variable scanThreadCV;
	std::mutex scanThreadCVMutex;

	void stopScanThread();
	void scanAndShow();

public:
	NeatocScanApp(neatoc::Controller *controller, bool useScanner = true, bool manualScanning = false, bool moveScanning = false);

	virtual bool OnInit();
	virtual int OnExit();

	void setMotorByKey(wxKeyEvent& keyEvent);
	void repaint(wxPaintEvent& paintEvent);

	static void Display(wxApp *app, int& argc, char **argv);
};

#endif // NEATOCSCANAPP_HPP
