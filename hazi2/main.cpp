//=============================================================================================
// Mintaprogram: Zold h·romszog. Ervenyes 2025.-tol
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni, ideÈrtve ChatGPT-t Ès t·rsait is
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

// cs˙cspont ·rnyalÛ
const char * vertSource = R"(
    #version 330
    precision highp float;

    layout(location = 0) in vec2 cP;
    uniform mat4 MVP;

    void main() {
        gl_Position = MVP * vec4(cP.x, cP.y, 0, 1);
    }
)";

// pixel ·rnyalÛ
const char * fragSource = R"(
	#version 330
    precision highp float;

	uniform vec3 color;			// konstans szÌn
	out vec4 fragmentColor;		// pixel szÌn

	void main() {
		fragmentColor = vec4(color, 1); // RGB -> RGBA
	}
)";
class Ball;

const int winWidth = 600, winHeight = 600;

class Camera {
    vec2 center;
    vec2 size;

public:
    Camera(vec2 c = vec2(0, 0), vec2 s = vec2(50, 50)) : center(c), size(s) {}

    vec2 getCenter() const { return center; }
    vec2 getSize()   const { return size; }

    void setCenter(const vec2& c) { center = c; }
    void setSize(const vec2& s)   { size = s; }

    float left()   const { return center.x - size.x / 2.0f; }
    float right()  const { return center.x + size.x / 2.0f; }
    float bottom() const { return center.y - size.y / 2.0f; }
    float top()    const { return center.y + size.y / 2.0f; }

    vec2 ScreenToWorld(int px, int py) const {
        float xNdc = 2.0f * px / winWidth - 1.0f;
        float yNdc = 1.0f - 2.0f * py / winHeight;

        vec4 pNdc(xNdc, yNdc, 0, 1);
        vec4 pWorld = ViewInverse() * ProjectionInverse() * pNdc;

        return vec2(pWorld.x, pWorld.y);
    }
    
    mat4 View() const {
        return mat4(
            1, 0, 0, -center.x,
            0, 1, 0, -center.y,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    mat4 Projection() const {
        return mat4(
            2.0f / size.x, 0, 0, 0,
            0, 2.0f / size.y, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    mat4 ViewProjection() const {
        return Projection() * View();
    }
    
    mat4 ViewInverse() const {
        return mat4(
            1, 0, 0, center.x,
            0, 1, 0, center.y,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    mat4 ProjectionInverse() const {
        return mat4(
            size.x / 2.0f, 0, 0, 0,
            0, size.y / 2.0f, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    
    mat4 ViewProjectionInverse() const {
        return ViewInverse() * ProjectionInverse();
    }
};


class Ball {
    vec2 pos0;
    vec2 vel0;
    float startTime;
    bool launched;
    float radius;

    Geometry<vec2>* geom;

    vec2 gravity;

public:
    float getStartTime(){
        return startTime;
    }
    
    vec2 GetPos0(){
        return pos0;
    }
    
    vec2 GetVel0(){
        return vel0;
    }
    
    vec2 GetGravity(){
        return gravity;
    }
    
    
    Ball() {
        pos0 = vec2(0, 0);
        vel0 = vec2(0, 0);
        startTime = 0.0f;
        launched = false;
        radius = 1.0f;
        gravity = vec2(0, -5.0f);

        geom = new Geometry<vec2>();
        BuildCircle();
    }

    void BuildCircle(int N = 40) {
        geom->Vtx().clear();

        geom->Vtx().push_back(vec2(0, 0));

        for (int i = 0; i <= N; i++) {
            float a = 2.0f * M_PI * i / N;
            geom->Vtx().push_back(vec2(cosf(a), sinf(a)) * radius);
        }

        geom->updateGPU();
    }

    bool IsLaunched() const { return launched; }

    void SetRadius(float r) {
        radius = r;
        BuildCircle();
    }

    float Radius() const { return radius; }

    void SetState(vec2 p, vec2 v, float t) {
        pos0 = p;
        vel0 = v;
        startTime = t;
    }

    void Launch(vec2 p, vec2 v, float t) {
        pos0 = p;
        vel0 = v;
        startTime = t;
        launched = true;
    }

    void Reset(vec2 p) {
        pos0 = p;
        vel0 = vec2(0, 0);
        startTime = 0.0f;
        launched = false;
    }

    vec2 PositionAt(float t) const {
        float dt = t - startTime;
        return pos0 + vel0 * dt + gravity * (0.5f * dt * dt);
    }

    vec2 VelocityAt(float t) const {
        float dt = t - startTime;
        return vel0 + gravity * dt;
    }

    vec2 CurrentPosition(float t) const {
        if (!launched) return pos0;
        return PositionAt(t);
    }

    vec2 CurrentVelocity(float t) const {
        if (!launched) return vec2(0, 0);
        return VelocityAt(t);
    }

    void Draw(GPUProgram* gpuProgram, const Camera& camera, float t) {
        vec2 c = CurrentPosition(t);

        mat4 M = mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            c.x, c.y, 0, 1
        );

        mat4 MVP = camera.ViewProjection() * M;
        gpuProgram->Use();
        gpuProgram->setUniform(MVP, "MVP");
        geom->Draw(gpuProgram, GL_TRIANGLE_FAN, vec3(0, 0, 1));
    }
};

class Spline {
    Geometry<vec2>* cps;
    Geometry<vec2>* curve;
    std::vector<float> ts;

    vec2 Hermite(vec2 p0, vec2 v0, float t0, vec2 p1, vec2 v1, float t1, float t) {
        float dt = t1 - t0;
        float u = t - t0;

        vec2 a0 = p0;
        vec2 a1 = v0;
        vec2 a2 = (p1 - p0) * (3.0f / (dt * dt)) - (v1 + v0 * 2.0f) / dt;
        vec2 a3 = (p0 - p1) * (2.0f / (dt * dt * dt)) + (v1 + v0) / (dt * dt);

        return ((a3 * u + a2) * u + a1) * u + a0;
    }

    void BuildKnots() {
        int n = (int)cps->Vtx().size();
        ts.clear();
        if (n == 0) return;

        ts.push_back(0.0f);
        for (int i = 0; i < n; i++) {
            vec2 a = cps->Vtx()[i];
            vec2 b = cps->Vtx()[(i + 1) % n];
            float d = length(b - a);
            ts.push_back(ts.back() + d);
        }
    }

    vec2 Velocity(int i) {
        int n = (int)cps->Vtx().size();

        int im1 = (i - 1 + n) % n;
        int ip1 = (i + 1) % n;

        float tim1, ti, tip1;

        ti = ts[i];

        if (i == 0) {
            tim1 = ts[0] - (ts[n] - ts[n - 1]);
        } else {
            tim1 = ts[i - 1];
        }

        if (i == n - 1) {
            tip1 = ts[n];
        } else {
            tip1 = ts[i + 1];
        }

        vec2 pim1 = cps->Vtx()[im1];
        vec2 pi   = cps->Vtx()[i];
        vec2 pip1 = cps->Vtx()[ip1];

        vec2 d1 = (pi - pim1) / (ti - tim1);
        vec2 d2 = (pip1 - pi) / (tip1 - ti);

        float w1 = (tip1 - ti) / (tip1 - tim1);
        float w2 = (ti - tim1) / (tip1 - tim1);

        return d1 * w1 + d2 * w2;
    }

public:
    Spline() {
        cps = new Geometry<vec2>();
        curve = new Geometry<vec2>();
    }

    float GetPeriod() const {
        if (ts.size() == 0) return 0.0f;
        return ts[ts.size() - 1];
    }

    Geometry<vec2>* GetCurve() {
        return curve;
    }

    void AddControlPoint(vec2 p) {
        cps->Vtx().push_back(p);
        cps->updateGPU();
        BuildKnots();
    }

    vec2 r(float t) {
        int n = (int)cps->Vtx().size();
        if (n == 0) return vec2(0, 0);
        if (n == 1) return cps->Vtx()[0];

        float period = ts[n];
        while (t < 0) t += period;
        while (t >= period) t -= period;

        for (int i = 0; i < n; i++) {
            float t0 = ts[i];
            float t1 = ts[i + 1];

            if (t0 <= t && t <= t1) {
                int i1 = (i + 1) % n;
                vec2 v0 = Velocity(i);
                vec2 v1 = Velocity(i1);
                return Hermite(cps->Vtx()[i], v0, t0, cps->Vtx()[i1], v1, t1, t);
            }
        }

        return cps->Vtx()[0];
    }

    void GenerateCurve() {
        curve->Vtx().clear();
        BuildKnots();

        int n = (int)cps->Vtx().size();
        if (n < 3) {
            curve->updateGPU();
            return;
        }

        float T = GetPeriod();
        int N = 100;

        for (int i = 0; i <= N; i++) {
            float t = T * i / N;
            curve->Vtx().push_back(r(t));
        }

        curve->updateGPU();
    }

    void Draw(GPUProgram* gpuProgram) {
        cps->Draw(gpuProgram, GL_POINTS, vec3(1,0,0));
        curve->Draw(gpuProgram, GL_LINE_STRIP,vec3(1,1,0));
    }
    
    bool collidesWith(float tstart, float tend, Ball* ball, vec2& hitPoint, vec2& hitNormal, float& hitTime) {
        std::vector<vec2>& pts = curve->Vtx();
        int n = (int)pts.size();
        if (n < 2) return false;

        bool found = false;
        float bestTime = tend + 1.0f;

        for (int i = 0; i < n - 1; i++) {
            vec2 A = pts[i];
            vec2 B = pts[i + 1];
            vec2 d = B - A;

            if (length(d) < 1e-8f) continue;

            vec2 normal(-d.y, d.x);

            float tau0 = tstart - ball->getStartTime();
            float tau1 = tend   - ball->getStartTime();

            vec2 p0 = ball->GetPos0();
            vec2 v0 = ball->GetVel0();
            vec2 g  = ball->GetGravity();

            float a = 0.5f * dot(g, normal);
            float b = dot(v0, normal);
            float c = dot(p0 - A, normal);

            float roots[2];
            int rootCount = 0;

            if (fabs(a) < 1e-8f) {
                if (fabs(b) > 1e-8f) {
                    roots[rootCount++] = -c / b;
                }
            } else {
                float D = b * b - 4.0f * a * c;
                if (D >= 0.0f) {
                    float sqrtD = sqrtf(D);
                    roots[rootCount++] = (-b - sqrtD) / (2.0f * a);
                    roots[rootCount++] = (-b + sqrtD) / (2.0f * a);
                }
            }

            for (int r = 0; r < rootCount; r++) {
                float tau = roots[r];
                if (tau < tau0 - 1e-8f || tau > tau1 + 1e-8f) continue;

                float tHit = ball->getStartTime() + tau;
                vec2 pHit = ball->PositionAt(tHit);

                float u = dot(pHit - A, d) / dot(d, d);
                if (u < -1e-8f || u > 1.0f + 1e-8f) continue;

                vec2 vHit = ball->VelocityAt(tHit);

                vec2 nrm = normalize(normal);
                if (dot(vHit, nrm) > 0.0f) nrm = -nrm;

                if (!found || tHit < bestTime) {
                    found = true;
                    bestTime = tHit;
                    hitTime = tHit;
                    hitPoint = A + d * u;
                    hitNormal = nrm;
                }
            }
        }

        return found;
    }
};


class BallApp : public glApp {
    Camera camera;
    GPUProgram* gpuProgram = nullptr;
    Spline* border = nullptr;
    Ball* ball = nullptr;
    float currentTime = 0.0f;
    bool launching = false;
    vec2 start;
    vec2 end;
    
public:
    BallApp() : glApp("BallBounce"), camera(vec2(0,0), vec2(50,50)) { }
    
	void onInitialization() override {
		gpuProgram = new GPUProgram(vertSource, fragSource);
        border = new Spline();
        ball = new Ball();
        
        glPointSize(10.0f);
        glLineWidth(3.0f);
	}

	// Ablak ˙jrarajzol·s
	void onDisplay() override {
		glClearColor(0, 0, 0, 0);     // h·ttÈr szÌn
		glClear(GL_COLOR_BUFFER_BIT); // rasztert·r tˆrlÈs
		//glViewport(0, 0, winWidth, winHeight);
        #ifdef __APPLE__
            // Retina screen support
            GLint vp[4];
            glGetIntegerv(GL_VIEWPORT, vp);
            glViewport(0, 0, vp[2], vp[3]);
        #else
            glViewport(0, 0, winWidth, winHeight);
        #endif
        
        gpuProgram->Use();
        mat4 mvp = camera.ViewProjection();
        gpuProgram->setUniform(mvp, "MVP");
        
        // TODO: use GL_POINT primitive
        border->Draw(gpuProgram);
        ball->Draw(gpuProgram, camera, currentTime);
	}
    
    void onMousePressed(MouseButton but, int pX, int pY) override {
        vec2 p = camera.ScreenToWorld(pX, pY);

        if (but == MOUSE_LEFT) {
            border->AddControlPoint(p);
            border->GenerateCurve();
        }
        
        if(but == MOUSE_RIGHT){
            start = p;
            launching = true;
        }

        refreshScreen();
    }
    
    void onMouseReleased(MouseButton but, int pX, int pY) override {
        const int CSUZLI_STRENGTH = 1;
        vec2 p = camera.ScreenToWorld(pX, pY);

        if(but == MOUSE_RIGHT && launching){
            end = p;
            
            vec2 v0 = (start - end) * CSUZLI_STRENGTH;
            launching = false;
            
            ball->Launch(end, v0, currentTime);
        }
        
        refreshScreen();
    }
    
    void onTimeElapsed(float tstart, float tend) override {
        if (!ball->IsLaunched()) {
                currentTime = tend;
                refreshScreen();
                return;
            }

        float simStart = tstart;
        const float timeEps = 1e-4f;
        int safety = 0;

        while (simStart < tend && safety < 20) {
            vec2 hp, hn;
            float ht;

            if (!border->collidesWith(simStart, tend, ball, hp, hn, ht)) break;

            vec2 v = ball->VelocityAt(ht);
            vec2 newV = v - 2.0f * dot(v, hn) * hn;
            vec2 newP = hp + hn * 1e-8f;

            ball->SetState(newP, newV, ht);
            simStart = ht + timeEps;

            safety++;
        }

        currentTime = tend;
        refreshScreen();
    }

    virtual void onKeyboard(int key) override {  }
    

};

BallApp app;

