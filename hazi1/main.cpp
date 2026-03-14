//=============================================================================================
// Mintaprogram: Zold háromszog. Ervenyes 2025.-tol
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni, ideértve ChatGPT-t és társait is
// - felesleges programsorokat a beadott programban hagyni 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL es GLM fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Uhrin Andor
// Neptun : GODO9K
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================//=============================================================================================
#include "framework.h"
#include "math.h"

enum SelectedType { NONE, LINE_T, CIRCLE_T };

// csúcspont árnyaló
const char * vertSource = R"(
	#version 330				
    precision highp float;

	layout(location = 0) in vec2 cP;	// 0. bemeneti regiszter

	void main() {
		gl_Position = vec4(cP.x, cP.y, 0, 1); 	// bemenet már normalizált eszközkoordinátákban
	}
)";

// pixel árnyaló
const char * fragSource = R"(
	#version 330
    precision highp float;

	uniform vec3 color;			// konstans szín
	out vec4 fragmentColor;		// pixel szín

	void main() {
		fragmentColor = vec4(color, 1); // RGB -> RGBA
	}
)";
class Circle;
class Line;

const int winWidth = 600, winHeight = 600;
const float eps = 0.02f;

class PointCollection{
    Geometry<vec3>* points;
    public:
    PointCollection(){
        points = new Geometry<vec3>();
    }
    
    void add(vec3 p){
        points->Vtx().push_back(p);
        printf("Point %.1f %.1f added\n", p.x, p.y);
        points->updateGPU();
    }
    
    void Draw(GPUProgram* prog){
        points->Draw(prog, GL_POINTS, vec3(1, 0, 0));
    }
    
    int findNearby(vec3 p){
        for(int i = 0; i<points->Vtx().size(); i++){
            vec3 s = points->Vtx()[i];
            vec3 d = p - s;
            if(d.x * d.x + d.y * d.y + d.z * d.z < eps * eps){
                return i;
            }
        }
        return -1;
    }
    
    Geometry<vec3>* getPoints() { return points; }
};

class Line {
    float a;
    float b;
    float c;
    Geometry<vec3>* endpoints;
    
    public:
    float getA() { return a; }
    float getB() { return b; }
    float getC() { return c; }
    Line(){}
    Line(vec3 s1, vec3 s2){
        //TODO: printout implicit and coordinates
        
        endpoints = new Geometry<vec3>();
        
        a = s1.y - s2.y;
        b = s2.x - s1.x;
        c = s1.x * s2.y - s2.x * s1.y;
        
        vec3 d = s2 - s1;
        printf("Line added\n");
        printf("\tImplicit: %.1f x + %.1f y + %.1f = 0\n", a, b, c);
        printf("\tParametric: r(t) = (%.1f, %.1f) + (%.1f, %.1f)t\n", s1.x, s1.y, d.x, d.y);
        if(fabs(a) > 1e-6f){
            float maxX = (-b-c)/a;
            float minX = (b-c)/a;
            endpoints->Vtx().push_back(vec3(minX, -1, 1));
            endpoints->Vtx().push_back(vec3(maxX, 1, 1));
        } else if(fabs(b) > 1e-6f){
            float y = (-a*s1.x - c)/b;
            endpoints->Vtx().push_back(vec3(-1, y, 1));
            endpoints->Vtx().push_back(vec3(1, y, 1));
        }
        endpoints->updateGPU();
    }
    
    Line(float a, float b, float c){
        this->a = a;
        this->b = b;
        this->c = c;

        endpoints = new Geometry<vec3>();

        if (fabs(b) > 1e-6f) {
            float y1 = (-a * (-1.0f) - c) / b;
            float y2 = (-a * ( 1.0f) - c) / b;
            endpoints->Vtx().push_back(vec3(-1.0f, y1, 1));
            endpoints->Vtx().push_back(vec3( 1.0f, y2, 1));
        }
        else if (fabs(a) > 1e-6f) {
            float x = -c / a;
            endpoints->Vtx().push_back(vec3(x, -1.0f, 1));
            endpoints->Vtx().push_back(vec3(x,  1.0f, 1));
        }
        endpoints->updateGPU();
    }
    
    void Draw(GPUProgram* prog){
        endpoints->Draw(prog, GL_LINES, vec3(0, 1, 1));
    }
    
    bool isPointOnLine(vec3 p){
        float r = fabs(a * p.x + b * p.y + c) / sqrt(a * a + b * b);
        return r < eps;
    }
    
    vec2* intersect(const Line& l){
        float d = a*l.b - b*l.a;
        if(fabs(d) < 1e-6f) return nullptr;
        float x = (b*l.c - c*l.b)/d;
        float y = (c*l.a - a*l.c)/d;
        return new vec2(x, y);
    }
};

class LineCollection {
    std::vector<Line> lines;
    public:
    void add(vec3 p1, vec3 p2){
        lines.push_back(Line(p1, p2));
    }
    
    std::vector<Line>& getLines() { return lines; }
    
    void Draw(GPUProgram* prog){
        for(int i=0; i<lines.size(); i++){
            lines[i].Draw(prog);
        }
    }
    
    int findLineNearby(vec3 p){
        for(int i=0; i<lines.size(); i++){
            if(lines[i].isPointOnLine(p)) return i;
        }
        return -1;
    }
};

class Circle {
    float r;
    vec3 center;
    Geometry<vec3>* points;
    
    void createGeometry(int segments = 100) {
        
        for (int i = 0; i <= segments; i++) {
            float t = 2.0f * M_PI * i / segments;
            float x = center.x + r * cos(t);
            float y = center.y + r * sin(t);
            points->Vtx().push_back(vec3(x, y, center.z));
        }
        points->updateGPU();
    }
    
    public:

    Circle(vec3 p1, vec3 p2, vec3 p3){
        // Kor eseten 3 egyenlet:
        // (u - x1)^2 - (v - y1)^2 = (u - x2)^2 - (v - y2)^2 = (u - x3)^2 - (v - y3)^2 = r^2
        // KŽt felezo egyenest keresŸnk a 3 pont kšzštt
        points = new Geometry<vec3>();
        float x1=p1.x, y1=p1.y;
        float x2=p2.x, y2=p2.y;
        float x3=p3.x, y3=p3.y;
        
        vec2 f1 = vec2((x1 + x2) / 2, (y1 + y2) / 2);
        vec2 f2 = vec2((x1 + x3) / 2, (y1 + y3) / 2);

        
        float a1 = x2 - x1;
        float b1 = y2 - y1;
        float c1 = -(a1 * f1.x + b1 * f1.y);

        Line l1(a1, b1, c1);

        
        float a2 = x3 - x1;
        float b2 = y3 - y1;
        float c2 = -(a2 * f2.x + b2 * f2.y);

        Line l2(a2, b2, c2);

        
        vec2* inter = l1.intersect(l2);
        if(inter == nullptr) return;

        center = vec3(inter->x, inter->y, 1);

        r = sqrt(
            (center.x - x1)*(center.x - x1) +
            (center.y - y1)*(center.y - y1)
        );

        createGeometry();

    }
    
    void Draw(GPUProgram* prog){
        points->Draw(prog, GL_LINE_LOOP, vec3(0, 1, 0));
    }
    
    bool isPointOnCircle(vec3 p) const {
        float dx = p.x - center.x;
        float dy = p.y - center.y;
        float dist = sqrt(dx * dx + dy * dy);
        return fabs(dist - r) < eps;
    }
    
    std::vector<vec3> intersect(Line l){
        std::vector<vec3> points;
        
        float a = l.getA(), b = l.getB(), c = l.getC();
        
        float x0, y0;

          // pont a vonalon
          if(fabs(b) > 1e-6f){
              x0 = 0;
              y0 = -c / b;
          } else {
              y0 = 0;
              x0 = -c / a;
          }

          // ir‡nyvektor
          float dx = -b;
          float dy = a;

          float fx = x0 - center.x;
          float fy = y0 - center.y;

          float A = dx*dx + dy*dy;
          float B = 2*(fx*dx + fy*dy);
          float C = fx*fx + fy*fy - r*r;

          float D = B*B - 4*A*C;

          if(D < -1e-6f) return points;

          if(fabs(D) < 1e-6f){
              float t = -B / (2*A);

              float x = x0 + dx*t;
              float y = y0 + dy*t;

              points.push_back(vec3(x,y,1));
              return points;
          }

          float sqrtD = sqrt(D);

          float t1 = (-B + sqrtD)/(2*A);
          float t2 = (-B - sqrtD)/(2*A);

          float x1 = x0 + dx*t1;
          float y1 = y0 + dy*t1;

          float x2 = x0 + dx*t2;
          float y2 = y0 + dy*t2;

          points.push_back(vec3(x1,y1,1));
          points.push_back(vec3(x2,y2,1));

          return points;
    }
    
    std::vector<vec3> intersect(const Circle& other){
        std::vector<vec3> pts;

        float x0 = center.x;
        float y0 = center.y;
        float r0 = r;

        float x1 = other.center.x;
        float y1 = other.center.y;
        float r1 = other.r;

        float dx = x1 - x0;
        float dy = y1 - y0;

        float d = sqrt(dx*dx + dy*dy);

        if(d > r0 + r1 + 1e-6f) return pts;      // nincs metszŽs
        if(d < fabs(r0 - r1) - 1e-6f) return pts; // egyik a m‡sikban
        if(d < 1e-6f) return pts;                // azonos kšzŽppont

        float a = (r0*r0 - r1*r1 + d*d) / (2*d);
        float h2 = r0*r0 - a*a;

        if(h2 < -1e-6f) return pts;
        if(h2 < 0) h2 = 0;

        float h = sqrt(h2);

        float xm = x0 + a * dx / d;
        float ym = y0 + a * dy / d;

        float rx = -dy * (h / d);
        float ry =  dx * (h / d);

        vec3 p1 = vec3(xm + rx, ym + ry, 1);
        vec3 p2 = vec3(xm - rx, ym - ry, 1);

        pts.push_back(p1);

        if(fabs(h) > 1e-6f) pts.push_back(p2);

        return pts;
    }

};

class CircleCollection{
    std::vector<Circle> circles;
    public:
    void add(vec3 p1, vec3 p2, vec3 p3){
        circles.push_back(Circle(p1, p2, p3));
    }
    
    void Draw(GPUProgram* prog){
        for(int i=0; i<circles.size(); i++){
            circles[i].Draw(prog);
        }
    }
    
    int findNearby(vec3 p){
        for(int i=0; i<circles.size(); i++){
            if(circles[i].isPointOnCircle(p)) return i;
        }
        return -1;
    }
    
    std::vector<Circle>& getCircles() { return circles; }
    
};

class PointsAndCirclesApp : public glApp {
	//Geometry<vec2>* triangle = nullptr;  // geometria
    PointCollection* pointList;
    LineCollection* lineList;
    CircleCollection* circleList;
    
    char mode = 'p';
    GPUProgram* gpuProgram = nullptr;	 // csúcspont és pixel árnyalók
    SelectedType selectedType = NONE;
    int selectedIndex = -1;
public:
    PointsAndCirclesApp() : glApp("Points And Circles") { }
    vec3 s1;
    vec3 s2;
    Line* l1 = nullptr;
    Circle* c1 = nullptr;
    
    bool presentL1 = false;
    bool presentC1 = false;
    bool presentS1 = false;
    bool presentS2 = false;
	// Inicializáció,
	void onInitialization() {
        //triangle = new Geometry<vec2>;
		gpuProgram = new GPUProgram(vertSource, fragSource);
        pointList = new PointCollection();
        lineList = new LineCollection();
        circleList = new CircleCollection();
        pointList->add(vec3(0.5f, 0.5f, 1));
        pointList->add(vec3(0.6f, 0.5f, 1));
        pointList->add(vec3(0.7f, 0.5f, 1));
        glPointSize(10.0f);
	}

	// Ablak újrarajzolás
	void onDisplay() {
		glClearColor(0, 0, 0, 0);     // háttér szín
		glClear(GL_COLOR_BUFFER_BIT); // rasztertár törlés
		//glViewport(0, 0, winWidth, winHeight);
        #define __APPLE__ 1
        #ifdef __APPLE__
            // Retina screen support
            GLint vp[4];
            glGetIntegerv(GL_VIEWPORT, vp);
            glViewport(0, 0, vp[2], vp[3]);
        #else
            glViewport(0, 0, winWidth, winHeight);
        #endif
        

        
        
        if(lineList) lineList->Draw(gpuProgram);
        if(circleList) circleList->Draw(gpuProgram);
        if(pointList) pointList->Draw(gpuProgram);

	}
    
    void onMousePressed(MouseButton but, int pX, int pY) {
        float x = 2.0f * pX / winWidth - 1.0f;
        float y = 1.0f - 2.0f * pY / winHeight;
        vec3 p = vec3(x, y, 1);
        int i = pointList->findNearby(p);
        vec3 currentPoint;
        if(i != -1) {
            currentPoint = pointList->getPoints()->Vtx()[i];
            printf("Current point: %.1f, %.1f\n", currentPoint.x, currentPoint.y);
        }
        
        switch(mode){
            case 'p':
                pointList->add(p);
                break;
            case 'c':
                if(i == -1) break;
                if(!presentS1) {
                    s1 = currentPoint;
                    presentS1 = true;
                    break;
                }
                if(!presentS2){
                    s2 = currentPoint;
                    presentS2 = true;
                    break;
                }
                
                circleList->add(s1, s2, currentPoint);
                presentS1 = false;
                presentS2 = false;
                
                break;
            case 'l':
                if(i == -1) break;
                if(!presentS1) {
                    s1 = currentPoint;
                    presentS1 = true;
                    break;
                }
                
                lineList->add(s1, currentPoint);
                presentS1 = false;
                break;
            case 'i':
                intersectMode(p);
                break;
            default: break;
        }
        

        refreshScreen();
    }
    
    void intersectMode(vec3 p){
        int l = lineList->findLineNearby(p);
          int c = circleList->findNearby(p);

          
          if(selectedType == NONE){
              if(l != -1){
                  selectedType = LINE_T;
                  selectedIndex = l;
                  return;
              }
              if(c != -1){
                  selectedType = CIRCLE_T;
                  selectedIndex = c;
                  return;
              }
              return;
          }

          
          if(selectedType == LINE_T){
              Line& firstLine = lineList->getLines()[selectedIndex];

              if(l != -1){
                  Line& secondLine = lineList->getLines()[l];

                  vec2* p = firstLine.intersect(secondLine);
                  if(p != nullptr){
                      pointList->add(vec3(p->x, p->y, 1));
                  }
              }
              else if(c != -1){
                  Circle& circle = circleList->getCircles()[c];

                  std::vector<vec3> pts = circle.intersect(firstLine);
                  for(int i = 0; i < pts.size(); i++){
                      pointList->add(pts[i]);
                  }
              }

              selectedType = NONE;
              selectedIndex = -1;
              return;
          }

          
          if(selectedType == CIRCLE_T){
              Circle& firstCircle = circleList->getCircles()[selectedIndex];

              if(l != -1){
                  Line& line = lineList->getLines()[l];

                  std::vector<vec3> pts = firstCircle.intersect(line);
                  for(int i = 0; i < pts.size(); i++){
                      pointList->add(pts[i]);
                  }
              }
              else if(c != -1){
                  Circle& secondCircle = circleList->getCircles()[c];

                  std::vector<vec3> pts = firstCircle.intersect(secondCircle);
                  for(int i = 0; i < pts.size(); i++){
                      pointList->add(pts[i]);
                  }
              }

              selectedType = NONE;
              selectedIndex = -1;
              return;
          }
    }
    
    virtual void onKeyboard(int key) {
        printf("Key: %c\n", key);
        mode = key;
        presentS1 = false;
        presentS2 = false;    }
    

};

PointsAndCirclesApp app;

