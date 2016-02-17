#include <vector>
#include <iostream>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <list>

/* header files for x functions*/
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

using namespace std;

const int Border = 5;
const int BufferSize = 10;
const int FPS = 30;

/* information to draw on the window */
struct XInfo {
	Display *display;
	int screen;
	Window window;
	GC  gc[3];
	
	Pixmap pixmap;		//double buffer
	int width;
	int height;
};


/* find a random number between low and high*/
int my_rand(int low, int high){
  return high + (low - high) * (double)rand() / RAND_MAX;

}

/*get a vector of random numbers between lower and upper in size size*/
vector<int> get_rand_n (int lower, int upper , int size) {
	int tmp;
	vector<int> result;
	int tmpr = size;	

	while( tmpr != 0) {
		tmp = my_rand(lower, upper);
		result.push_back(tmp);
		tmpr --;
	}
	return result;
}

/*Function to put out a message on error exits */
void error( string str) {
	cerr << str << endl;
	exit(0);
}

/*
Setup the background and lines for the game
*/
void SetupX(int i, XInfo &xInfo) {
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	if( i == 0 ) {
		XSetForeground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
		XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
	}
	else if ( i == 1 || i == 2) {
		XSetForeground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
		XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	}
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	if( i == 0 || i == 1) {
		XSetLineAttributes(xInfo.display, xInfo.gc[i], 1, LineSolid, CapButt, JoinRound);
	}
	else if ( i == 2) {
		XSetLineAttributes(xInfo.display, xInfo.gc[i], 10, LineSolid, CapButt, JoinRound);
	}
}

/*
get the index of int where x[i] > p
otherwise return the size+1
*/
int FindInterval(vector<int> &x, float p) {
	int i = 0;
	for(; i <= x.size(); i++) {
		if(i == x.size()) {
			return i;
		}
		else if(p < x[i]) {
			return i;
		}
	}
	return i;
}

/*
find a vector of ints that is greater than low
and smaller than high
*/
vector<int> findPoints (float low, float high , vector<int> &x) {
	vector<int> set;
	for(int i = 0 ; i < x.size() ; i++) {
		if(x[i] >= low && x[i] <= high) {
			set.push_back(i);
		}
	}
	return set;
}

/*
return true if any points smaller or eaqual to y.
false is all greater than y
*/
bool lower ( vector<int> points , float y) {
	for(int i = 0 ; i < points.size() ; i++) {
		if( y >= points[i]) {
			return true;
		}
	}
	return false;
}
//=======DEFINE CLASS=========
/* an abstract class representing displayable things */
class Displayable {
	public :
		virtual void paint (XInfo &xinfo) = 0;
};

class Bomb : public Displayable {
	public:
		virtual void paint(XInfo &xinfo) {
			
			// the center circle
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x-10, y-25, diameter, diameter, 0, 360*64);
			//vertical shoot light
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x-60, y-25, diameter+100, diameter-50, 0, 360*64);
			//horizontal shoot light
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x-10, y-75, diameter-50, diameter+100, 0, 360*64);
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x-500, y-75, diameter-50, diameter+100, 0, 360*64);
		}

		void reset(int shipx, int shipy) {
			x = shipx;
			y = shipy;
		}
		
		Bomb(int x, int y, int diameter): x(x), y(y), diameter(diameter) {
		}
	
	private:
		int x;
		int y;
		int diameter;
};

class Ship : public Displayable {
	public:
		virtual void paint (XInfo &xinfo) {
			
			// ship body
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y+20, x+40, y+20);		//bottom line
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y+20, x+10, y+10);		//left bottom slop line
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+40, y+20, x+30, y+10);		//right bottom slop line
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+10, y+10, x+30, y+10);		//middle line
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+10, y+10, x+15, y);		//left top slop line
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+30, y+10, x+25, y);		//right top slop line
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+15, y, x+25, y);		//top line
			
			//flag
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+20, y, x+20, y-10);		//the vertial bar at the top
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+20, y-10, 5, 5, 0, 360*64);		//the flag
			
			// dots on the ship
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+30, y+15, 5, 5, 0, 360*64);	
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+12, y+15, 5, 5, 0, 360*64);
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+5, y+15, 5, 5, 0, 360*64);
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+24, y+15, 5, 5, 0, 360*64);
		}
		void move(XInfo &xinfo,int &left , int &right , int &up, int &down) {		
			y = y + gravityc;
			y = y + gravity;
			x = x + velocity;

			gravityc = 1.006 * gravityc;
			velocity = velocity / 1.025;
			gravity = gravity / 1.025;

			if(y > 580 || y > xinfo.width - gravity ) {
				gravityc = 0;
			}
			else if (left > 0 ) {
				velocity = velocity - movement;
				x = x + velocity;
				left--;
			}
			else if (right > 0 ) {
				velocity = velocity + movement;
				x = x + velocity;
				right --;
			}
			else if ( up > 0 ) {
				gravity = gravity - movement;
				y = y + gravity;
				gravityc = 0.75;
				up--;
			}
			else if (down > 0 ) {
				gravity = gravity + movement;
				y = y + gravity;
				down--;
			}
		}
		
		int getX() {
			return x;
		}
		
		int getY() {
			return y+20;
		}

		float getspeed() {
			return gravity + gravityc;
		}
			
		void reset() {	
			diameter = rdiameter;
			x = rx;
			y = ry;
			gravity = 0;
			gravityc = 0.75;  
			movement = 0.1;
			velocity = 0;
		}		

		Ship( int diameter, float x, float y):diameter(diameter), x(x), y(y) {
			gravity = 0;
			gravityc = 0.9;
			movement = 0.1;
			velocity = 0;
		}
		
		private:
			float x;
			float y;
			int diameter;
			float rx;			//the original x
			float ry;			//the original y
			int rdiameter;
			float gravity;
			float gravityc;		//the gravity constant
			float velocity;
			float movement;
};

class Mountain: public Displayable {
	public:
		virtual void paint (XInfo &xinfo) {
			int i = 0 ;		
			int index = peak;
			XDrawLine(xinfo.display , xinfo.pixmap,xinfo.gc[1],leftx ,lefty ,x[0],y[0]);
			while(index != 1 ) {
				XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x[i], y[i], x[i+1], y[i+1]);
				index --;
				i ++;
			}
			XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x[i],y[i],rightx, righty);
		}
		
		vector<int> getx() {
			return x;
		}
		
		vector<int> gety() {
			return y;
		}
		
		void reset(int lx, int ly , int rx, int ry) {
			leftx = lx;
			lefty = ly;
			rightx = rx;
			righty = ry;
			x = get_rand_n(leftx,rightx,peak);
			sort(x.begin(),x.end());
			y = get_rand_n(100,600,peak);
		}

		Mountain(int peak,int leftx, int lefty, int rightx, int righty ) :peak(peak), 
					leftx(leftx), lefty(lefty),rightx(rightx),righty(righty){ 
			x = get_rand_n(leftx,rightx,peak);
			sort(x.begin(),x.end());
			y = get_rand_n(100,600,peak);

		}
		
		private:
			vector<int> x;
			vector<int> y;
			int leftx;
			int lefty;
			int rightx;
			int righty;
			int peak;
};

class Pad : public Displayable {
	public:
		virtual void paint (XInfo &xinfo) {	
			XDrawLine(xinfo.display , xinfo.pixmap , xinfo.gc[2] , x , y, x+l , y);
		}

		void reset() {
			x = my_rand(left,right);
			y = my_rand(300,400);
		}

		int getx() {
			return x;
		}
		int gety() {
			return y;
		}
		int getl() {
			return l;
		}
	
		Pad(int l, int left, int right):l(l), left(left), right(right) {
			rl = l;
			rleft = left;
			rright = right;
			x = my_rand(left,right);
			y = my_rand(300,400);
		}
		
		private :
			int x;
			int y;
			int l;
			int left;
			int right;
			int rl;
			int rleft;
			int rright;
};

class Text : public Displayable {
	public:
	    virtual void paint(XInfo& xinfo) {
	        XDrawImageString( xinfo.display, xinfo.pixmap, xinfo.gc[2],
	                          this->x, this->y, this->s.c_str(), this->s.length() );
	    }

	    // constructor
	    Text(int x, int y, string s):x(x), y(y), s(s) {}

	private:
	    XPoint p; 
	    int x;
	    int y;
	    string s; // string to show
};

//===============================================================================

list<Displayable *> dList;
Ship ship(40,0,0);
Pad pad(50,100,200);
Pad pad2(50, 500, 600);
int leftx = pad.getx()+pad.getl();  // the right x-ixs of the first pad
int lefty = pad.gety(); 	//the right y-ixs of the first pad
int rightx = pad2.getx();  // the left x-ixs of the second pad
int righty = pad2.gety();  // the left y-ixs of the second pad

//draw mountains
Mountain mountain1(10,0,500, pad.getx(), pad.gety());  
Mountain mountain3(10,leftx,lefty,rightx,righty);
Mountain mountain2(10, pad2.getx()+pad2.getl(),pad2.gety(),800,0);
Bomb bomb(ship.getX(), ship.getY(), 40);

void initX(int argc, char* argv[], XInfo &xInfo){	
	XSizeHints hints;
	unsigned long white, black;

	xInfo.display = XOpenDisplay( "" );
	if( !xInfo.display ) {
		error ( "Can't open display." );
	}
	
	xInfo.screen = DefaultScreen( xInfo.display );

	white = XWhitePixel( xInfo.display, xInfo.screen );
	black = XBlackPixel( xInfo.display, xInfo.screen );

	hints.x = 100;
	hints.y = 100;
	hints.width = 800;
	hints.height = 600;
	hints.flags = PPosition | PSize |PMinSize;

	xInfo.window = XCreateSimpleWindow (
		xInfo.display,
		DefaultRootWindow( xInfo.display ),
		hints.x , hints.y,
		hints.width , hints.height,
		Border,
		black,
		black );

	XSetStandardProperties(
		xInfo.display,
		xInfo.window,
		"Lander",
		"XLander",
		None,
		argv, argc , 
		&hints );
	
	/* create graphics contexts*/
	int i = 0;
	SetupX(i, xInfo);

	// Reverse Video
	i = 1;
	SetupX(i, xInfo);
	
	//thicker than 1
	i = 2;
	SetupX(i, xInfo);
						 
	int depth = DefaultDepth(xInfo.display , DefaultScreen(xInfo.display));
	xInfo.pixmap = XCreatePixmap(xInfo.display , xInfo.window , hints.width, hints.height , depth);
	xInfo.width = hints.width;
	xInfo.height = hints.height;
	

	XSelectInput(xInfo.display , xInfo.window , 
			KeyPressMask |KeyReleaseMask | StructureNotifyMask);  //key press and resize

	/* put the window on the screen */
	XMapRaised( xInfo.display , xInfo.window );

	XFlush(xInfo.display);
	sleep(2);
}

/* 
function to repaint a display list 
Checking collisions with mountain
NOTE:
all the conditions for mountains are different
cannot use helper function to reduce the code
*/
int repaint(XInfo &xinfo) {
	list<Displayable *>::const_iterator begin = dList.begin();
	list<Displayable *>::const_iterator end = dList.end();
	int status = 1;	  //1: nothing happened 0:landed 2: crushed
	float slop;	
	int ship_point;			
	int i;
	float length;
	float height;
	float expecty;
	int extra;
	int real_y;
	vector<int> x;
	vector<int> y;

	//big black rectangle to clear background
	XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0], 0 , 0 , xinfo.width, xinfo.height);

	//draw display list
	while (begin != end) {
		Displayable *d = *begin;
		d->paint(xinfo);
		begin++;
	}
	
	//check if the ship landed
	if ( (ship.getY() >= lefty && pad.getx() <= ship.getX()+40 && ship.getX() <= leftx ) ||
	     (ship.getY() >= righty && rightx <= ship.getX()+40 && ship.getX() < rightx+pad2.getl()) ) {
		if(ship.getspeed() > 2) {	
			status = 2;
		} else {
		status = 0;
		}
	}

	if(ship.getX()  < pad.getx() ) {
		/*mountain 1*/
		x = mountain1.getx();	
		y = mountain1.gety();
		vector<int> indexs = findPoints(ship.getX(), ship.getX()+40, x);
		vector<int> points;
		for(int j = 0; j < indexs.size(); j ++ ) {
			points.push_back(y[indexs[j]]);
		}

		real_y = ship.getY();
		
		// right point
		if(ship.getX() +40 < pad.getx()) {
			ship_point = ship.getX()+40;
			i = FindInterval( x, ship_point);
			if ( i == 0 ) {
				length = x[i] - 0;
				height = y[i] - 500;
				slop = height / length;
				extra = 500;
			}
			else if( i == x.size() ) {
				length = pad.getx() - x[i-1];
				height = pad.gety() - y[i-1];
				slop = height / length;
				extra = y[i-1] - x[i-1]*slop;
			}
			else {
				length = x[i] - x[i-1];
				height = y[i] - y[i-1];
				slop = height / length;
				extra = y[i] - x[i]*slop;		
			} 
			expecty = ship_point * slop +extra;
			if ( real_y >= expecty) {
				status = 2;
			} 
		}
		// left point
		if( status == 1) {
			ship_point = ship.getX();
			i = FindInterval( x, ship_point);
			if ( i == 0 ) {
				length = x[i] - 0;
				height = y[i] - 500;
				slop = height / length;
				extra = 500;
			}
			else if( i == x.size() ) {
				length = pad.getx() - x[i-1];
				height = pad.gety() - y[i-1];
				slop = height / length;
				extra = y[i-1] - x[i-1]*slop;
			}	
			else {
				length = x[i] - x[i-1];
				height = y[i] - y[i-1];
				slop = height / length;
				extra = y[i] - x[i]*slop;		
			} 
			expecty = ship_point * slop +extra;
			if ( real_y >= expecty) {
				status = 2;
			}
		} 
		//line 
		if(lower(points, ship.getY()) && status == 1) {
			status = 2;
		} 
	}
	else if( ship.getX()  < pad2.getx() && ship.getX()+40 > pad.getx() + pad.getl() ) {
		//mountain 3
		x = mountain3.getx();
		y = mountain3.gety();
		vector<int> indexs = findPoints(ship.getX(), ship.getX()+40, x);
		vector<int> points;
		for(int j = 0; j < indexs.size(); j ++ ) {
			points.push_back(y[indexs[j]]);
		}

		real_y = ship.getY();

		//right point
		if(ship.getX() +40 > pad.getx() +pad.getl() && ship.getX() +40 < pad2.getx()) {
			ship_point = ship.getX()+40;
			i = FindInterval( x, ship_point);
			if ( i == 0 ) {
				length = x[i] - pad.getx()+pad.getl();
				height = y[i] - pad.gety();
				slop = height / length;
				extra = y[i] - x[i] *slop;
			}
			else if( i == x.size() ) {
				length = pad2.getx() - x[i-1];
				height = pad2.gety() - y[i-1];
				slop = height / length;
				extra = y[i-1] - x[i-1]*slop;
			}
			else {
				length = x[i] - x[i-1];
				height = y[i] - y[i-1];
				slop = height / length;
				extra = y[i] - x[i]*slop;		
			}
			expecty = ship_point * slop +extra;
			if ( real_y >= expecty) {
				status = 2;
			}
		}
		//left point
		if(ship.getX() < pad2.getx() && ship.getX() > pad.getx() + pad.getl()) {
			if(status == 1) {
				ship_point = ship.getX();
				i = FindInterval( x, ship_point);
				if ( i == 0 ) {
					length = x[i] - pad.getx()+pad.getl();
					height = y[i] - pad.gety();
					slop = height / length;
					extra = pad.gety() - (pad.getx()+pad.getl()) *slop;
				}
				else if( i == x.size() ) {
					length = pad2.getx() - x[i-1];
					height = pad2.gety() - y[i-1];
					slop = height / length;
					extra = y[i-1] - x[i-1]*slop;
				}
				else {
					length = x[i] - x[i-1];
					height = y[i] - y[i-1];
					slop = height / length;
					extra = y[i] - x[i]*slop;		
				}
				expecty = ship_point * slop +extra;
				cout << "real_y " << real_y << endl;
				cout << "expecty" << expecty << endl;
				if ( real_y >= expecty) {
					status = 2;
				}
			}
		}
		//line
		if(lower(points,ship.getY()) && status == 1) {
			status = 2;
		}
	}
	else if( ship.getX() +40< 800 && ship.getX()+40 > pad2.getx() + pad2.getl() ) {
		x = mountain2.getx();
		y = mountain2.gety();
		vector<int> indexs = findPoints(ship.getX(), ship.getX()+40, x);
		vector<int> points;
		for(int j = 0; j < indexs.size(); j ++ ) {
			points.push_back(y[indexs[j]]);
		}

		real_y = ship.getY();

		//right point
			ship_point = ship.getX()+40;
			i = FindInterval( x, ship_point);
			if ( i == 0 ) {
				length = x[i] - pad2.getx()+pad.getl();
				height = y[i] - pad2.gety();
				slop = height / length;
				extra = y[i] - x[i] *slop;
			}
			else if( i == x.size() ) {
				length = 0 - x[i-1];
				height = 800 - y[i-1];
				slop = height / length;
				extra = y[i-1] - x[i-1]*slop;
			}
			else {
				length = x[i] - x[i-1];
				height = y[i] - y[i-1];
				slop = height / length;
				extra = y[i] - x[i]*slop;		
			} 
			expecty = ship_point * slop +extra;
			if ( real_y >= expecty) {
				status = 2;
			}
		

		//left point
		if(ship.getX() > pad2.getx() &&ship.getX() < 800) {
		if(status == 1) {
			ship_point = ship.getX();
			i = FindInterval( x, ship_point);
			if ( i == 0 ) {
				length = x[i] - pad2.getx()+pad2.getl();
				height = y[i] - pad2.gety();
				slop = height / length;
				extra = pad2.gety() - (pad2.getx()+pad2.getl()) *slop;
			}
			else if( i == x.size() ) {
				length = 0 - x[i-1];
				height = 800 - y[i-1];
				slop = height / length;
				extra = y[i-1] - x[i-1]*slop;
			}
			else {
				length = x[i] - x[i-1];
				height = y[i] - y[i-1];
				slop = height / length;
				extra = y[i] - x[i]*slop;		
			} 
			expecty = ship_point * slop +extra;
			if ( real_y >= expecty) {
				status = 2;
			}
		}
		}
		//line
		if(lower(points, ship.getY()) && status == 1) {
			status = 2;
		}
	}
	
	XCopyArea(xinfo.display , xinfo.pixmap, xinfo.window , xinfo.gc[0],
			0,0,xinfo.width,xinfo.height,(xinfo.width-800)/2,(xinfo.height-600)/2);
	
	XFlush(xinfo.display);
	return status;
}

void handleAnimation(XInfo &xinfo, int& left , int & right , int & up , int & down , int &enable) {
	if(enable == 1 || up != 0 ) {
		enable = 1;
		ship.move(xinfo , left, right , up , down);
	}
}

void handleKeyPress(XInfo &xinfo, XEvent &event, int &left, int &right , int & up, int & down , int & pause , int & start) {
	KeySym key;
	char text[BufferSize];

	int i = (int) XLookupKeysym (&event.xkey, 0);
	if ( i == 65361) {
		cout << "left" << endl;
		if(start != 0 ){
			dList.pop_front();
			dList.push_front(new Text(122, 25, "left") );
		}
		left ++;
	}
	else if( i == 65362 ) {	
		cout << "up" << endl;
		if(start != 0 ){
			dList.pop_front();
			dList.push_front(new Text(122, 25, "up") );
		}
		up ++;
	}
	else if( i == 65363 ) {
		cout << "right" << endl;
		if(start != 0 ){
			dList.pop_front();
			dList.push_front(new Text(122, 25, "right") );
		}
		right++;
	}
	else if( i == 65364 ) {
		cout << "down" << endl;
		if(start != 0 ){
			dList.pop_front();
			dList.push_front(new Text(122, 25, "down") );
		}
		down++;
	}	
	else if(i == 32) {
		//pause when start is 1, continue when start is 0
		if(start == 0 ) {
			dList.pop_front();
			dList.push_front(&ship);
			dList.push_front(&mountain1);
			dList.push_front(&mountain2);
			dList.push_front(&mountain3);
			dList.push_front(&pad);
			dList.push_front(&pad2);
			dList.push_front(new Text(50, 50, "space to pause") );
			dList.push_front(new Text(50, 25, "you pressed ") );
			dList.push_front(new Text(50, 25, " ") );
			start = 1;
		}
		else  {
			dList.pop_front();
			dList.push_front(new Text(122, 25, "space") );
			int p=0;
			while ( p != 32) {
				XNextEvent(xinfo.display , &event);
				if(event.type == KeyPress) {
					p = (int) XLookupKeysym (&event.xkey, 0);
					cout << "pause" << endl;
				}
			}
		}
	}
	else if( i == 113) {
		start = 3;
	}
}

void handleResize(XInfo &xinfo , XEvent &event, int &small) {
	XConfigureEvent xce = event.xconfigure;
	fprintf(stderr, "handling resize w=%d h=%d\n", xce.width, xce.height);
	
	//check if the window gets too small 
	if(xce.width < 800 || xce.height <600 ) {
		dList.push_front(new Text(xce.width/2, 100, "Too Small") );
		cout << "too small" << small << endl;
		small ++;
		int crush = repaint(xinfo);
	}

	else if(small != 0) {
	//cout << small << endl;
		while( small != 0 ) {
			dList.pop_front();
			small --;
		}
	}
	
	//resize the window
	if(xce.width != xinfo.width || xce.height != xinfo.height) {
		int depth = DefaultDepth(xinfo.display, DefaultScreen(xinfo.display));
		xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, xce.width, xce.height, depth);
		xinfo.width = xce.width;
		xinfo.height = xce.height;
	}
}

//get microseconds
unsigned long now() {
	timeval tv;
	gettimeofday(&tv , NULL );
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

int eventLoop(XInfo &xinfo, int start) {
	dList.push_front(new Text(xinfo.width/2 - 100, xinfo.height/2, "press space to start") );
	
	XEvent event;
	unsigned long lastRepaint = 0;
	int small = 0;
	int left = 0;
	int right = 0;
	int up = 0;
	int down = 0;
	int pause = 0;
	int enable = 0;

	while(true) {
		if(XPending(xinfo.display) > 0 ) {
			XNextEvent(xinfo.display , &event);
			switch (event.type) {
				case KeyPress:
					handleKeyPress(xinfo, event,left,right,up,down,pause , start);
					enable = 1;
					break;
				case ConfigureNotify:
					handleResize(xinfo,event,small);
					break;
			}
		}
		
		//animation timing
		unsigned long end = now();
		if ( end - lastRepaint > 1000000 / FPS ) {
			handleAnimation(xinfo, left, right , up , down , enable);
			int status = repaint(xinfo);
			if( status == 0 ) {
				enable = 0;
				cout << "land" << endl;
				dList.push_front(new Text(xinfo.width/2 - 100, 50, "YOU WIN") );
				dList.push_front(new Text(xinfo.width/2 - 100, 100, "press \"SPACE\" to restart") );
				dList.push_front(new Text(xinfo.width/2 - 100, 150, "press \"q\" to quit") ); 
				status = repaint(xinfo);
				int p = 0;
				while ( p != 32 && p != 113) {
					XNextEvent(xinfo.display , &event);
					if(event.type == KeyPress) {
						p = (int) XLookupKeysym (&event.xkey, 0);
					}
				}
				if(p == 32) {
					return 1;
				}
				else {
					return 0;
				}
				
			}
			else if(status == 2) {
				enable = 0;
				bomb.reset(ship.getX(), ship.getY());
				dList.push_front(&bomb);
				dList.push_front(new Text(xinfo.width/2 - 100, 50, "GAME OVER") );
				dList.push_front(new Text(xinfo.width/2 - 100, 100, "press \"space\" to restart") );
				dList.push_front(new Text(xinfo.width/2 - 100, 150, "press \"q\"to quit") ); 
				status = repaint(xinfo);
				int p = 0; 
				// 32 for space, 113 for "q"
				while ( p != 32 && p != 113) {
					XNextEvent(xinfo.display , &event);
					switch (event.type) {
						case KeyPress:
							p = (int) XLookupKeysym (&event.xkey, 0);
						case ConfigureNotify:
							break; 
					}
				}
				if(p == 32) {
					return 1;
				}
				else {
					return 0;
				}
				
			}
			if(start == 3) {
				enable = 0;
				cout << "quit" << endl;
				return 0;
			}
			lastRepaint = now();
		}

		//give the system time to do other things
		else if(XPending(xinfo.display) == 0 ){
			usleep(1000000/FPS - (end - lastRepaint));
		}
	}
}

int main( int argc , char*argv[] ) {
	int start = 0 ; 
	XInfo xInfo;
	initX(argc, argv, xInfo);
	
	while(true) {
		int i = eventLoop(xInfo, 0);
		ship.reset();
		pad.reset();
		pad2.reset();

		leftx = pad.getx()+pad.getl();  // the right x-ixs of the first pad
		lefty = pad.gety(); 	//the right y-ixs of the first pad
		rightx = pad2.getx();  // the left x-ixs of the second pad
		righty = pad2.gety();  // the left y-ixs of the second pad
		mountain1.reset(0,500, pad.getx(), pad.gety());  
		mountain3.reset(leftx,lefty,rightx,righty);
		mountain2.reset( pad2.getx()+pad2.getl(),pad2.gety(),800,0);
		while(dList.size() > 0 ) {
			dList.pop_front();
		}
		if( i == 0 ){
			break;
		}
		start = 0;
	}
	
	XCloseDisplay(xInfo.display);
}
