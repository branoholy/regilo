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

RegiloVisual::RegiloVisual(regilo::IScanController *controller, bool useScanner, bool manualScanning, bool moveScanning, double orientation) : wxApp(),
	controller(controller), useScanner(useScanner), manualScanning(manualScanning), moveScanning(moveScanning),
	orientation(orientation), safeDistanceW(400), safeDistanceH(30), dangerPart(0), emergency(true),
	fullscreen(false), zoom(0.08),
	radarColor(0, 200, 0), pointColor(200, 200, 200), radarAngle(orientation), radarRayLength(4000)
{
	safeBox = wxRect(0, 0, 1300 + 2 * safeDistanceW, 750 + 2 * safeDistanceH);

	if(useScanner)
	{
		/*
		if(controller->getLog() != nullptr)
		{
			moveController.setLog(controller->getLog());
		}
		*/

		moveController.connect("192.168.1.100:12346");
		moveController.REQUEST_END = "";
		moveController.readResponse = false;
	}
}

bool RegiloVisual::OnInit()
{
	wxPathList pathList;
	pathList.Add(".");
	pathList.Add(wxStandardPaths::Get().GetResourcesDir());

	wxInitAllImageHandlers();
	radarGradient.LoadFile(pathList.FindValidPath("images/radar-gradient.png"));
	radarGradientZoom = zoomImage(radarGradient, zoom * 10);

	car.LoadFile(pathList.FindValidPath("images/car-black.png"));
	carZoom = zoomImage(car, zoom);

	// Frame
	frame = new wxFrame(NULL, wxID_ANY, "Regilo Visual", wxDefaultPosition, wxSize(600, 400));

	// Frame StatusBar
	frame->CreateStatusBar(2, wxSTB_ELLIPSIZE_MIDDLE | wxSTB_SHOW_TIPS | wxFULL_REPAINT_ON_RESIZE);

	std::string endpoint;
	if(useScanner) endpoint = controller->getEndpoint();
	else endpoint = controller->getLog()->getFilePath();

	setStatusText("", 0);
	setStatusText("Connected to " + endpoint, 1);

	// Panel
	panel = new wxPanel(frame);
	panel->GetEventHandler()->Bind(wxEVT_KEY_UP, &RegiloVisual::setMotorByKey, this);
	panel->GetEventHandler()->Bind(wxEVT_PAINT, &RegiloVisual::repaint, this);
	panel->GetEventHandler()->Bind(wxEVT_LEFT_DCLICK, [this] (wxMouseEvent&)
	{
		if((zoom * 2) > 2) return;

		zoom *= 2;
		radarGradientZoom = zoomImage(radarGradient, zoom * 10);
		carZoom = zoomImage(car, zoom);

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
		carZoom = zoomImage(car, zoom);

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
	frame->Maximize(true);

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
			setStatusText("Move scanning...", 0);
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
				setStatusText("Going up...", 0);

				if(keyEvent.ControlDown()) neatoController->setMotor(500, 500, 100);
				else neatoController->setMotor(100, 100, 50);

				setStatusText("Going up... Done!", 0);
				controllerMutex.unlock();
			}
			else if(moveController.isConnected())
			{
				moveControllerMutex.lock();
				setStatusText("Going up...", 0);
				moveController.sendCommand("w");
				setStatusText("Going up... Done!", 0);
				direction++;
				moveControllerMutex.unlock();
			}
			break;

		case WXK_DOWN:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				setStatusText("Going down...", 0);

				neatoController->setMotor(-100, -100, 50);

				setStatusText("Going down... Done!", 0);
				controllerMutex.unlock();
			}
			else if(moveController.isConnected())
			{
				moveControllerMutex.lock();
				setStatusText("Going down...", 0);
				moveController.sendCommand("x");
				setStatusText("Going down... Done!", 0);
				direction--;
				moveControllerMutex.unlock();
			}
			break;

		case WXK_LEFT:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				setStatusText("Turning left...", 0);

				if(keyEvent.ControlDown()) neatoController->setMotor(-30, 30, 50);
				else neatoController->setMotor(20, 100, 50);

				setStatusText("Turning left... Done!", 0);
				controllerMutex.unlock();
			}
			else if(moveController.isConnected())
			{
				moveControllerMutex.lock();
				setStatusText("Turning left...", 0);
				if(keyEvent.ControlDown()) moveController.sendCommand("a");
				else moveController.sendCommand("A");
				setStatusText("Turning left... Done!", 0);
				moveControllerMutex.unlock();
			}
			break;

		case WXK_RIGHT:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				setStatusText("Turning right...", 0);

				if(keyEvent.ControlDown()) neatoController->setMotor(30, -30, 50);
				else neatoController->setMotor(100, 20, 50);

				setStatusText("Turning right... Done!", 0);
				controllerMutex.unlock();
			}
			else if(moveController.isConnected())
			{
				moveControllerMutex.lock();
				setStatusText("Turning right...", 0);
				if(keyEvent.ControlDown()) moveController.sendCommand("d");
				else moveController.sendCommand("D");
				setStatusText("Turning right... Done!", 0);
				moveControllerMutex.unlock();
			}
			break;

		case WXK_SPACE:
			if(neatoController != nullptr)
			{
				controllerMutex.lock();
				setStatusText("Stopping...", 0);

				neatoController->setMotor(0, 0, 0);

				setStatusText("Stopping... Done!", 0);
				controllerMutex.unlock();
			}
			else if(moveController.isConnected())
			{
				moveControllerMutex.lock();
				setStatusText("Stopping...", 0);
				moveController.sendCommand("s");
				setStatusText("Stopping... Done!", 0);
				direction = 0;
				moveControllerMutex.unlock();
			}
			break;

		case 'S':
			if(manualScanning)
			{
				setStatusText("Manual scanning...", 0);
				scanAndShow();
			}
			break;

		case 'E':
			emergency = !emergency;
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

	// wxBitmap bufferBitmap(panel->GetSize());
	// wxBufferedPaintDC dc(panel);
	// dc.SelectObject(bufferBitmap);

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

	// Draw car
	wxPoint carPos;
	wxRect rotatedSafeBox;
	wxImage rotatedCarZoom = carZoom.Rotate(orientation, wxPoint(carZoom.GetWidth() / 2, carZoom.GetHeight() / 2));
	if(orientation == 0)
	{
		rotatedSafeBox.SetSize(wxSize(zoom * safeBox.GetWidth(), zoom * safeBox.GetHeight()));
		rotatedSafeBox.SetPosition(wxPoint(width2 - zoom * (150 + safeDistanceW), height2 - zoom * safeBox.GetHeight() / 2));
		carPos.x = width2 - zoom * 150;
		carPos.y = height2 - rotatedCarZoom.GetHeight() / 2;
	}
	else
	{
		rotatedSafeBox.SetSize(wxSize(zoom * safeBox.GetHeight(), zoom * safeBox.GetWidth()));
		rotatedSafeBox.SetPosition(wxPoint(width2 - zoom * safeBox.GetHeight() / 2, height2 - zoom * (safeBox.GetWidth() - 150 - safeDistanceW)));
		carPos.x = width2 - rotatedCarZoom.GetWidth() / 2;
		carPos.y = height2 - (rotatedCarZoom.GetHeight() - zoom * 150);
	}
	dc.SetPen(*wxRED_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(rotatedSafeBox);

	if(dangerPart != 0)
	{
		dc.SetBrush(*wxRED_BRUSH);
		if(dangerPart == 1)
		{
			if(orientation == 0)
			{
				rotatedSafeBox.width *= 0.5;
				rotatedSafeBox.x += rotatedSafeBox.width;
			}
			else
			{
				rotatedSafeBox.height *= 0.5;
			}
		}
		else if(dangerPart == -1)
		{
			if(orientation == 0)
			{
				rotatedSafeBox.width *= 0.5;
			}
			else
			{
				rotatedSafeBox.height *= 0.5;
				rotatedSafeBox.y += rotatedSafeBox.height;
			}
		}
		dc.DrawRectangle(rotatedSafeBox);
	}

	dc.DrawBitmap(rotatedCarZoom, carPos);

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

	if(!emergency)
	{
		std::string message = "EMERGENCY STOP: OFF";

		dc.SetTextForeground(*wxRED);
		dc.SetFont(*wxTheFontList->FindOrCreateFont(40, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

		wxSize messageSize = dc.GetTextExtent(message);
		dc.DrawText(message, width2 - messageSize.GetWidth() / 2, 100);
	}

/*
	if(bufferBitmap.IsOk())
	{
		static int id = 0;

		std::ostringstream oss;
		oss << std::setw(4) << std::setfill('0') << id++;

		bufferBitmap.SaveFile("lidar-animace/lidar-" + oss.str() + ".png", wxBITMAP_TYPE_PNG);
	}
*/
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

	if(emergency) emergencyStop();

	controllerMutex.unlock();

	this->GetTopWindow()->GetEventHandler()->CallAfter([this, emptyData] ()
	{
		if(emptyData) setStatusText("No more scans to show (EOF).", 0);
		else frame->Refresh();
	});
}

void RegiloVisual::emergencyStop()
{
	wxRect posSafeBox = safeBox;
	posSafeBox.SetPosition(wxPoint(-150 - safeDistanceW, - safeBox.GetHeight() / 2));

	dangerPart = 0;
	for(const regilo::ScanRecord& record : data)
	{
		if(record.error) continue;

		double x = record.distance * std::cos(record.angle);
		double y = record.distance * std::sin(record.angle);

		if(posSafeBox.Contains(x, y))
		{
			double centerX = posSafeBox.GetLeft() + posSafeBox.GetWidth() / 2;

			if((direction > 0 && x > centerX) || (direction < 0 && x < centerX))
			{
				if(direction > 0 && x > centerX) dangerPart = 1;
				if(direction < 0 && x < centerX) dangerPart = -1;

				std::cout << "Emergency STOP!" << std::endl;

				if(moveController.isConnected())
				{
					moveControllerMutex.lock();

					this->GetTopWindow()->GetEventHandler()->CallAfter([this]()
					{
						setStatusText("Emergency STOP...", 0);
					});

					moveController.sendCommand("s");
					direction = 0;

					this->GetTopWindow()->GetEventHandler()->CallAfter([this]()
					{
						setStatusText("Emergency STOP... Done!", 0);
					});

					moveControllerMutex.unlock();
				}

				break;
			}
		}
	}
}

void RegiloVisual::setStatusText(const std::string& text, int i)
{
	frame->SetStatusText(text, i);
	if(i == 1) refreshStatusBar();
}

void RegiloVisual::refreshStatusBar()
{
	wxStatusBar *statusBar = frame->GetStatusBar();

	int lastWidth = statusBar->GetTextExtent(statusBar->GetStatusText(1)).GetWidth() + 10;
	if(lastWidth > 400) lastWidth = 400;

	const int statusBarWidths[] = { -1, lastWidth };
	statusBar->SetStatusWidths(2, statusBarWidths);
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
