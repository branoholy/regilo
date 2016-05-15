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

#include "regilovisual.hpp"

#include <chrono>

#include <wx/dcgraph.h>
#include <wx/stdpaths.h>

#include <regilo/neatocontroller.hpp>
#include <regilo/serialcontroller.hpp>
#include <regilo/socketcontroller.hpp>

RegiloVisual::RegiloVisual(regilo::IScanController *controller, bool useScanner, bool manualScanning, bool moveScanning) : wxApp(),
	controller(controller), useScanner(useScanner), manualScanning(manualScanning), moveScanning(moveScanning),
	fullscreen(false), zoom(0.08),
	radarColor(0, 200, 0), pointColor(200, 200, 200), radarAngle(0), radarRayLength(4000)
{
}

bool RegiloVisual::OnInit()
{
	wxPathList pathList;
	pathList.Add(".");
	pathList.Add(wxStandardPaths::Get().GetResourcesDir());

	wxInitAllImageHandlers();
	radarGradient.LoadFile(pathList.FindValidPath("images/radar-gradient.png"));
	radarGradientZoom = zoomImage(radarGradient, zoom * 10);

	// Frame
	frame = new wxFrame(NULL, wxID_ANY, "Regilo Visual", wxDefaultPosition, wxSize(600, 400));

	// Frame StatusBar
	wxStatusBar *statusBar = frame->CreateStatusBar(2);

	std::string endpoint;
	if(useScanner) endpoint = controller->getEndpoint();
	else endpoint = controller->getLog()->getFilePath();

	frame->SetStatusText("", 0);
	frame->SetStatusText("Connected to " + endpoint, 1);

	int sbWidths[] = { -1, 300 };
	statusBar->SetStatusWidths(2, sbWidths);

	// Panel
	panel = new wxPanel(frame);
	panel->GetEventHandler()->Bind(wxEVT_KEY_UP, &RegiloVisual::setMotorByKey, this);
	panel->GetEventHandler()->Bind(wxEVT_PAINT, &RegiloVisual::repaint, this);
	panel->GetEventHandler()->Bind(wxEVT_LEFT_DCLICK, [this] (wxMouseEvent&)
	{
		if((zoom * 2) > 2) return;

		zoom *= 2;
		radarGradientZoom = zoomImage(radarGradient, zoom * 10);

		int width, height;
		panel->GetSize(&width, &height);
		double maxWidth = std::sqrt(std::pow(width / 2, 2) + std::pow(height / 2, 2));

		if(radarGradientZoom.GetWidth() > maxWidth)
		{
			double scale = maxWidth / radarGradientZoom.GetWidth();
			radarGradientZoom.Resize(radarGradientZoom.GetSize() * scale, wxPoint());
		}
	});
	panel->GetEventHandler()->Bind(wxEVT_RIGHT_DCLICK, [this] (wxMouseEvent&)
	{
		if((zoom * 0.5) < 0.002) return;

		zoom *= 0.5;
		radarGradientZoom = zoomImage(radarGradient, zoom * 10);

		int width, height;
		panel->GetSize(&width, &height);
		double maxWidth = std::sqrt(std::pow(width / 2, 2) + std::pow(height / 2, 2));

		if(radarGradientZoom.GetWidth() > maxWidth)
		{
			double scale = maxWidth / radarGradientZoom.GetWidth();
			radarGradientZoom.Resize(radarGradientZoom.GetSize() * scale, wxPoint());
		}
	});

	if(!manualScanning && !moveScanning)
	{
		scanThreadRunning = true;
		scanThread = std::thread([this] ()
		{
			while(scanThreadRunning)
			{
				scanAndShow();

				if(scanThreadRunning)
				{
					std::unique_lock<std::mutex> lock(scanThreadCVMutex);
					scanThreadCV.wait_for(lock, std::chrono::milliseconds(500));
				}
			}
		});
	}

	std::size_t fps = 24;
	radarThread = std::thread([this, fps] ()
	{
		while(scanThreadRunning)
		{
			radarMutex.lock();
			radarAngle += M_PI / fps / 2;
			radarMutex.unlock();

			this->GetTopWindow()->GetEventHandler()->CallAfter([this] ()
			{
				frame->Refresh();
			});

			if(scanThreadRunning)
			{
				std::unique_lock<std::mutex> lock(radarThreadCVMutex);
				radarThreadCV.wait_for(lock, std::chrono::milliseconds(1000 / fps));
			}
		}
	});

	frame->Show(true);

	return true;
}

int RegiloVisual::OnExit()
{
	stopScanThread();
	if(scanThread.joinable()) scanThread.join();
	if(radarThread.joinable()) radarThread.join();

	return wxApp::OnExit();
}

void RegiloVisual::setMotorByKey(wxKeyEvent& keyEvent)
{
	int keyCode = keyEvent.GetKeyCode();

	if(keyCode == WXK_UP || keyCode == WXK_DOWN || keyCode == WXK_LEFT || keyCode == WXK_RIGHT)
	{
		if(moveScanning)
		{
			frame->SetStatusText("Move scanning...", 0);
			scanAndShow();
		}
	}

	regilo::INeatoController *neatoController = dynamic_cast<regilo::NeatoController<regilo::SocketController>*>(controller);
	if(neatoController == nullptr) neatoController = dynamic_cast<regilo::NeatoController<regilo::SerialController>*>(controller);

	switch(keyCode)
	{
		case WXK_UP:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				frame->SetStatusText("Going up...", 0);

				if(keyEvent.ControlDown()) neatoController->setMotor(500, 500, 100);
				else neatoController->setMotor(100, 100, 50);

				frame->SetStatusText("Going up... Done!", 0);
				controllerMutex.unlock();
			}
			break;

		case WXK_DOWN:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				frame->SetStatusText("Going down...", 0);

				neatoController->setMotor(-100, -100, 50);

				frame->SetStatusText("Going down... Done!", 0);
				controllerMutex.unlock();
			}
			break;

		case WXK_LEFT:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				frame->SetStatusText("Turning left...", 0);

				if(keyEvent.ControlDown()) neatoController->setMotor(-30, 30, 50);
				else neatoController->setMotor(20, 100, 50);

				frame->SetStatusText("Turning left... Done!", 0);
				controllerMutex.unlock();
			}
			break;

		case WXK_RIGHT:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				frame->SetStatusText("Turning right...", 0);

				if(keyEvent.ControlDown()) neatoController->setMotor(30, -30, 50);
				else neatoController->setMotor(100, 20, 50);

				frame->SetStatusText("Turning right... Done!", 0);
				controllerMutex.unlock();
			}
			break;

		case WXK_SPACE:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				frame->SetStatusText("Stopping...", 0);

				neatoController->setMotor(0, 0, 0);

				frame->SetStatusText("Stopping... Done!", 0);
				controllerMutex.unlock();
			}
			break;

		case 'S':
			if(manualScanning)
			{
				frame->SetStatusText("Manual scanning...", 0);
				scanAndShow();
			}
			break;

		case WXK_F11:
			fullscreen = !fullscreen;
			frame->ShowFullScreen(fullscreen);
			break;

		case WXK_ESCAPE:
			fullscreen = false;
			frame->ShowFullScreen(fullscreen);
			break;

		default:
			keyEvent.Skip();
	}
}

wxImage RegiloVisual::zoomImage(const wxImage& image, double zoom)
{
	wxImage zoomedImage = image;
	zoomedImage.Rescale(int(image.GetWidth() * zoom), int(image.GetHeight() * zoom));

	return zoomedImage;
}

wxRect RegiloVisual::getRotatedBoundingBox(const wxRect& rect, double angle)
{
	double c = std::cos(angle);
	double s = std::sin(angle);

	wxPoint minBound(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	wxPoint maxBound(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());

	wxPoint points[] = { rect.GetLeftTop(), rect.GetLeftBottom(), rect.GetRightTop(), rect.GetRightBottom() };
	for(wxPoint& point : points)
	{
		int x = int(std::ceil(c * point.x - s * point.y));
		int y = int(std::ceil(s * point.x + c * point.y));

		if(x < minBound.x) minBound.x = x;
		if(y < minBound.y) minBound.y = y;
		if(x > maxBound.x) maxBound.x = x;
		if(y > maxBound.y) maxBound.y = y;

		point.x = x;
		point.y = y;
	}

	wxRect box;
	box.SetLeftTop(minBound);
	box.SetRightBottom(maxBound);

	return box;
}

void RegiloVisual::drawRadarGradient(wxDC& dc, int width2, int height2)
{
	wxImage rotatedImage = radarGradientZoom.Rotate(radarAngle, wxPoint());

	wxRect box(radarGradientZoom.GetSize());
	box.width++;
	box.height++;
	wxRect rotatedBox = getRotatedBoundingBox(box, -radarAngle);

	wxPoint offset = rotatedBox.GetLeftTop();
	offset.x += width2 - 1;
	offset.y += height2 - 1;

	dc.DrawBitmap(wxBitmap(rotatedImage), offset);
}

void RegiloVisual::repaint(wxPaintEvent&)
{
	wxPaintDC dc(panel);
	wxGCDC gcdc(dc);

	// Draw backgroud
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawRectangle(panel->GetSize());

	int width, height;
	panel->GetSize(&width, &height);

	int width2 = width / 2;
	int height2 = height / 2;

	// Draw axis
	dc.SetPen(*wxThePenList->FindOrCreatePen(radarColor, 2));
	dc.DrawLine(0, height2, width, height2);
	dc.DrawLine(width2, 0, width2, height);

	// Draw circles
	gcdc.SetPen(*wxThePenList->FindOrCreatePen(radarColor, 2));
	gcdc.SetBrush(*wxTRANSPARENT_BRUSH);
	for(std::size_t radius = 1000; radius <= 4000; radius += 1000)
	{
		gcdc.DrawCircle(width2, height2, int(radius * zoom));
	}

	// Draw radar ray
	radarMutex.lock();

	double rayLength = zoom * radarRayLength;
	double maxRayLength = std::sqrt(width2 * width2 + height2 * height2);
	if(rayLength > maxRayLength) rayLength = maxRayLength;

	int radarLineX = int(width2 + rayLength * std::cos(radarAngle));
	int radarLineY = int(height2 - rayLength * std::sin(radarAngle));
	gcdc.DrawLine(width2, height2, radarLineX, radarLineY);

	drawRadarGradient(dc, width2, height2);

	radarMutex.unlock();

	controllerMutex.lock();

	dc.SetPen(*wxThePenList->FindOrCreatePen(pointColor));
	for(const regilo::ScanRecord& record : data)
	{
		if(record.error) continue;

		double distance = record.distance * zoom;
		int x = int(width2 + distance * std::cos(record.angle));
		int y = int(height2 - distance * std::sin(record.angle));

		dc.DrawRectangle(x, y, 2, 2);
	}

	controllerMutex.unlock();
}

void RegiloVisual::stopScanThread()
{
	if(scanThreadRunning)
	{
		scanThreadRunning = false;
		scanThreadCV.notify_one();
		radarThreadCV.notify_one();
	}
}

void RegiloVisual::scanAndShow()
{
	controllerMutex.lock();

	data = controller->getScan(useScanner);
	bool emptyData = data.empty();
	if(emptyData) stopScanThread();

	controllerMutex.unlock();

	this->GetTopWindow()->GetEventHandler()->CallAfter([this, emptyData] ()
	{
		if(emptyData) frame->SetStatusText("No more scans to show (EOF).", 0);
		else frame->Refresh();
	});
}

void RegiloVisual::Display(wxApp *app, int& argc, char **argv)
{
	wxApp::SetInstance(app);
	wxEntryStart(argc, argv);
	app->CallOnInit();
	app->OnRun();
	app->OnExit();
	wxEntryCleanup();
}
