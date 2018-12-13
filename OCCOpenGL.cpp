#include "OCCOpenGL.h"

#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QStyleFactory>


#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_Transform.hxx>


#ifdef WNT
    #include <WNT_Window.hxx>
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
    #include <Cocoa_Window.hxx>
#else
    #undef Bool
    #undef CursorShape
    #undef None
    #undef KeyPress
    #undef KeyRelease
    #undef FocusIn
    #undef FocusOut
    #undef FontChange
    #undef Expose
    #include <Xw_Window.hxx>
#endif


static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
  static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
  return aGraphicDriver;
}

OCCOpenGL::OCCOpenGL(QWidget* parent )
    : QGLWidget(parent),
    myXmin(0),
    myYmin(0),
    myXmax(0),
    myYmax(0),    
    myCurrentMode(CurAction3d_DynamicRotation),
    myDegenerateModeIsOn(Standard_True),
    myRectBand(NULL),
	interactiveMode(InterMode::Viewing)
{
	helm = new PHELM();

    // No Background
    setBackgroundRole( QPalette::NoRole );

    // Enable the mouse tracking, by default the mouse tracking is disabled.
    setMouseTracking( true );
}

void OCCOpenGL::init()
{
    // Create Aspect_DisplayConnection
    Handle(Aspect_DisplayConnection) aDisplayConnection =
            new Aspect_DisplayConnection();

    // Get graphic driver if it exists, otherwise initialize it
    if (GetGraphicDriver().IsNull())
    {
        GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
    }

    // Get window handle. This returns something suitable for all platforms.
    WId window_handle = (WId) winId();

    // Create appropriate window for platform
    #ifdef WNT
        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle) window_handle);
    #elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
        Handle(Cocoa_Window) wind = new Cocoa_Window((NSView *) window_handle);
    #else
        Handle(Xw_Window) wind = new Xw_Window(aDisplayConnection, (Window) window_handle);
    #endif

    // Create V3dViewer and V3d_View
    myViewer = new V3d_Viewer(GetGraphicDriver(), Standard_ExtString("viewer3d"));

    myView = myViewer->CreateView();

    myView->SetWindow(wind);
    if (!wind->IsMapped()) wind->Map();

    // Create AISInteractiveContext
    myContext = new AIS_InteractiveContext(myViewer);

    // Set up lights etc
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();

    myView->SetBackgroundColor(Quantity_NOC_WHITE);
    myView->MustBeResized();
	myView->SetShadingModel(Graphic3d_TypeOfShadingModel::V3d_PHONG);
	myView->SetZoom(50);
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_BLACK, 0.08, V3d_ZBUFFER);

    myContext->SetDisplayMode(AIS_Shaded, Standard_True);
	myContext->MainSelector()->SetPickClosest(Standard_False);
	
	// face selection
	/*
	myContext->CloseAllContexts(Standard_True);
	myContext->OpenLocalContext();
	myContext->ActivateStandardMode(TopAbs_FACE);
	*/

	aManipulator = new AIS_Manipulator();
	//aManipulator->SetPart(1, AIS_ManipulatorMode::AIS_MM_Rotation, Standard_False);
	aManipulator->SetModeActivationOnDetection(Standard_True);

}

const Handle(AIS_InteractiveContext)& OCCOpenGL::getContext() const
{
    return myContext;
}

void OCCOpenGL::paintEvent( QPaintEvent* /*theEvent*/ )
{
    if (myContext.IsNull())
    {
        init();
    }

    myView->Redraw();
}

void OCCOpenGL::resizeEvent( QResizeEvent* /*theEvent*/ )
{
    if( !myView.IsNull() )
    {
        myView->MustBeResized();
    }
}

void OCCOpenGL::fitAll( void )
{
    myView->FitAll();
    myView->ZFitAll();
    myView->Redraw();
}

void OCCOpenGL::reset( void )
{
    myView->Reset();
}

void OCCOpenGL::pan( void )
{
    myCurrentMode = CurAction3d_DynamicPanning;
}

void OCCOpenGL::zoom( void )
{
    myCurrentMode = CurAction3d_DynamicZooming;
}

void OCCOpenGL::rotate( void )
{
    myCurrentMode = CurAction3d_DynamicRotation;
}

void OCCOpenGL::mousePressEvent( QMouseEvent* theEvent )
{
    if (theEvent->button() == Qt::LeftButton)
    {
        onLButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
    else if (theEvent->button() == Qt::MidButton)
    {
        onMButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton)
    {
        onRButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
}

void OCCOpenGL::mouseReleaseEvent( QMouseEvent* theEvent )
{
	if (theEvent->button() == Qt::LeftButton)
	{
		
		onLButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());


		auto numSelected = myContext->NbSelected();
		if (interactiveMode == InterMode::Manipulating &&
			numSelected == 1 &&
			aManipulator->Object().get() != myContext->SelectedInteractive().get())
		{
			
		}
		else
		{
			myContext->InitSelected();
		}


		if ((theEvent->buttons() | theEvent->modifiers()) & Qt::ControlModifier)
		{
			if (aManipulator->IsAttached()) aManipulator->Detach();
			
			if (numSelected > 1)
			{
				vecShapes.clear();
				while (myContext->MoreSelected())
				{
					if (myContext->HasSelectedShape())
					{
						auto selectedInteractiveObj = myContext->SelectedInteractive();
						vecShapes.emplace_back(selectedInteractiveObj);
					}
					myContext->NextSelected();
				}
			}
			std::cout << "vecshapes " << vecShapes.size() << std::endl;
			// If CTRL pressed, only execute it
			return;
		}

		if (numSelected == 1 && interactiveMode == InterMode::Viewing)
		{
			while (myContext->MoreSelected())
			{
				if (myContext->HasSelectedShape())
				{
					auto selectedInteractiveObj = myContext->SelectedInteractive();
					AIS_Manipulator::BehaviorOnTransform behavior;
					behavior.SetFollowRotation(Standard_True);
					behavior.SetFollowTranslation(Standard_True);
					aManipulator->SetTransformBehavior(behavior);
					aManipulator->Attach(selectedInteractiveObj);
				}
				myContext->NextSelected();
			}
			
			// Enable manipulate mode
			interactiveMode = InterMode::Manipulating;
		}

		if (numSelected == 0 && interactiveMode == InterMode::Manipulating)
		{
			interactiveMode = InterMode::Viewing;
			
			// Update internal model
			
			aManipulator->Detach();
			aManipulator->SetModeActivationOnDetection(Standard_True);
			myView->Redraw();
		}

		if (numSelected == 1 && interactiveMode == InterMode::Manipulating && aManipulator->HasActiveTransformation())
		{
			{
				Handle(AIS_Shape) selectedShape = Handle(AIS_Shape)::DownCast(aManipulator->Object());
				std::cout << aManipulator->IsModeActivationOnDetection() << std::endl;
				BRepBuilderAPI_Transform trsf(selectedShape->Shape(), manipulatorTrsf);
				
				auto updatedShape = trsf.Shape();
				selectedShape->Set(updatedShape);

				// TODO: Get full transformation
				gp_XYZ transPart = manipulatorTrsf.TranslationPart();
				helm->transformShape(selectedShape, transPart);
			}

			aManipulator->StopTransform(Standard_True);
			aManipulator->SetModeActivationOnDetection(Standard_True);

		}
		
    }
    else if (theEvent->button() == Qt::MidButton)
    {
        onMButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton)
    {
		return;
		inputEvent(theEvent->pos().x(), theEvent->pos().y());

		// select an object if it can be changed by parameters
		pairParaShape.second = false;
		myContext->InitSelected();

		auto numSelected = myContext->NbSelected();
		std::cout << "nb selected: = " << numSelected << std::endl;
		if (numSelected != 0)
		{
			while (myContext->MoreSelected())
			{
				if (myContext->HasSelectedShape())
				{
					auto selectedInteractiveObj = myContext->SelectedInteractive();
					pairParaShape.first = selectedInteractiveObj;
					pairParaShape.second = true;
				}
				myContext->NextSelected();
			}
		}

        onRButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
}

void OCCOpenGL::mouseMoveEvent( QMouseEvent * theEvent )
{
    onMouseMove(theEvent->buttons(), theEvent->pos());
}

void OCCOpenGL::wheelEvent( QWheelEvent * theEvent )
{
    onMouseWheel(theEvent->buttons(), theEvent->delta(), theEvent->pos());
}

void OCCOpenGL::onLButtonDown( const int /*theFlags*/, const QPoint thePoint )
{
    // Save the current mouse coordinate in min.
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();
	std::cout << "Mode " << aManipulator->ActiveMode() << std::endl;
	if (aManipulator->HasActiveMode())
	{
		aManipulator->SetModeActivationOnDetection(Standard_False);
		aManipulator->StartTransform(myXmin, myYmin, myView);
		std::cout << "started" << std::endl;
	}
}

void OCCOpenGL::onMButtonDown( const int /*theFlags*/, const QPoint thePoint )
{
   
}

void OCCOpenGL::onRButtonDown( const int /*theFlags*/, const QPoint thePoint )
{
	// Save the current mouse coordinate in min.
	myXmin = thePoint.x();
	myYmin = thePoint.y();
	myXmax = thePoint.x();
	myYmax = thePoint.y();

	if (myCurrentMode == CurAction3d_DynamicRotation)
	{
		myView->StartRotation(thePoint.x(), thePoint.y());
	}
}

void OCCOpenGL::onLButtonUp( const int theFlags, const QPoint thePoint )
{
    // Hide the QRubberBand
    if (myRectBand)
    {
        myRectBand->hide();
    }

    // Ctrl for multi selection.
    if (thePoint.x() == myXmin && thePoint.y() == myYmin)
    {
        if (theFlags & Qt::ControlModifier)
        {
            multiInputEvent(thePoint.x(), thePoint.y());
        }
        else
        {
            inputEvent(thePoint.x(), thePoint.y());
        }
    }


}

void OCCOpenGL::onMButtonUp( const int /*theFlags*/, const QPoint thePoint )
{

	if (thePoint.x() == myXmin && thePoint.y() == myYmin)
	{
		panByMiddleButton(thePoint);
	}

}

void OCCOpenGL::onRButtonUp( const int /*theFlags*/, const QPoint thePoint )
{
	QMenu menu;

	if (pairParaShape.second)
	{
		QAction* paramAct = new QAction("Set its parameters", this);
		menu.addAction(paramAct);
	}
	QAction* openAct = new QAction("Open...", this);
	menu.addAction(openAct);
	menu.addSeparator();
	menu.exec(mapToGlobal(thePoint));
}

void OCCOpenGL::onMouseMove( const int theFlags, const QPoint thePoint )
{
    // Draw the rubber band.
    if (theFlags & Qt::LeftButton)
    {
		if (interactiveMode == InterMode::Manipulating && aManipulator->HasActiveMode())
		{
			manipulatorTrsf = aManipulator->Transform(thePoint.x(), thePoint.y(), myView);
			myView->Redraw();
		}
		else
		{
			drawRubberBand(myXmin, myYmin, thePoint.x(), thePoint.y());

			dragEvent(thePoint.x(), thePoint.y());

		}
    }

    // Ctrl for multi selection.
    if (theFlags & Qt::ControlModifier)
    {
        multiMoveEvent(thePoint.x(), thePoint.y());
    }
    else
    {
        moveEvent(thePoint.x(), thePoint.y());
    }

    // Middle button.
    if (theFlags & Qt::RightButton)
    {
        switch (myCurrentMode)
        {
        case CurAction3d_DynamicRotation:
            myView->Rotation(thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicZooming:
            myView->Zoom(myXmin, myYmin, thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicPanning:
            myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
            myXmax = thePoint.x();
            myYmax = thePoint.y();
            break;

         default:
            break;
        }
    }

}


void OCCOpenGL::onMouseWheel(const int /*theFlags*/, const int theDelta, const QPoint thePoint)
{
	Standard_Integer aFactor = 16;

	Standard_Integer aX = thePoint.x();
	Standard_Integer aY = thePoint.y();

	if (theDelta > 0)
	{
		aX += aFactor;
		aY += aFactor;
	}
	else
	{
		aX -= aFactor;
		aY -= aFactor;
	}

	myView->Zoom(thePoint.x(), thePoint.y(), aX, aY);
}

void OCCOpenGL::addItemInPopup(QMenu* /*theMenu*/)
{
	
}


void OCCOpenGL::dragEvent( const int x, const int y )
{
    myContext->Select(myXmin, myYmin, x, y, myView, Standard_True);

	emit selectionChanged();
}

void OCCOpenGL::multiDragEvent( const int x, const int y )
{
    myContext->ShiftSelect(myXmin, myYmin, x, y, myView, Standard_True);

    emit selectionChanged();

}

void OCCOpenGL::inputEvent( const int x, const int y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
	myContext->Select(Standard_True);


    emit selectionChanged();
}

void OCCOpenGL::multiInputEvent( const int x, const int y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    myContext->ShiftSelect(Standard_True);

    emit selectionChanged();
}

void OCCOpenGL::moveEvent( const int x, const int y )
{

    myContext->MoveTo(x, y, myView, Standard_True);

}

void OCCOpenGL::multiMoveEvent( const int x, const int y )
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void OCCOpenGL::drawRubberBand( const int minX, const int minY, const int maxX, const int maxY )
{
    QRect aRect;

    // Set the rectangle correctly.
    (minX < maxX) ? (aRect.setX(minX)) : (aRect.setX(maxX));
    (minY < maxY) ? (aRect.setY(minY)) : (aRect.setY(maxY));

    aRect.setWidth(abs(maxX - minX));
    aRect.setHeight(abs(maxY - minY));

    if (!myRectBand)
    {
        myRectBand = new QRubberBand(QRubberBand::Rectangle, this);

        // setStyle is important, set to windows style will just draw
        // rectangle frame, otherwise will draw a solid rectangle.
        myRectBand->setStyle(QStyleFactory::create("windows"));
    }

    myRectBand->setGeometry(aRect);
    myRectBand->show();
}

void OCCOpenGL::panByMiddleButton( const QPoint& thePoint )
{
	fitAll();
	return;

    Standard_Integer aCenterX = 0;
    Standard_Integer aCenterY = 0;

    QSize aSize = size();

    aCenterX = aSize.width() / 2;
    aCenterY = aSize.height() / 2;

    myView->Pan(aCenterX - thePoint.x(), thePoint.y() - aCenterY);
}


void OCCOpenGL::fuse_selected()
{
	if (vecShapes.size() != 2) return;

	if (aManipulator->HasActiveMode()) aManipulator->Detach();

	if (vecShapes[0]->IsKind(STANDARD_TYPE(AIS_Shape)) && vecShapes[1]->IsKind(STANDARD_TYPE(AIS_Shape)))
	{
		Handle(AIS_Shape) shapeA = Handle(AIS_Shape)::DownCast(vecShapes[0]);
		Handle(AIS_Shape) shapeB = Handle(AIS_Shape)::DownCast(vecShapes[1]);

		TopoDS_Shape aFusedShape = BRepAlgoAPI_Fuse(shapeA->Shape(), shapeB->Shape());
		Handle(AIS_Shape) anAisCylinder = new AIS_Shape(aFusedShape);
		std::cout << "fused done" << std::endl;
		myContext->Remove(vecShapes[0], Standard_True);
		myContext->Remove(vecShapes[1], Standard_True);
		myContext->Display(anAisCylinder, Standard_True);
	}

}

void OCCOpenGL::intersect_selected()
{
	if (vecShapes.size() != 2) return;

	if (aManipulator->HasActiveMode()) aManipulator->Detach();

	if (vecShapes[0]->IsKind(STANDARD_TYPE(AIS_Shape)) && vecShapes[1]->IsKind(STANDARD_TYPE(AIS_Shape)))
	{
		Handle(AIS_Shape) shapeA = Handle(AIS_Shape)::DownCast(vecShapes[0]);
		Handle(AIS_Shape) shapeB = Handle(AIS_Shape)::DownCast(vecShapes[1]);

		TopoDS_Shape aFusedShape = BRepAlgoAPI_Cut(shapeA->Shape(), shapeB->Shape());
		Handle(AIS_Shape) intersectedSahpe = new AIS_Shape(aFusedShape);
		std::cout << "cut done" << std::endl;
		myContext->Remove(vecShapes[0], Standard_True);
		myContext->Remove(vecShapes[1], Standard_True);
		myContext->Display(intersectedSahpe, Standard_True);

		helm->intersectAwithB(shapeA, shapeB, intersectedSahpe);
	}
}
