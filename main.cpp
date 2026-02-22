#include <QCoreApplication>
#include <iostream>
#include <vector>
#include <cstddef>
#include <sstream>
#include <string>

// ====================== 1. Prototype ======================
class GraphObject {
protected:
    bool isColored;

public:
    GraphObject(bool colored = true) : isColored(colored) {}
    virtual ~GraphObject() = default;

    virtual GraphObject* clone() const = 0;
    virtual void draw() const = 0;
    virtual size_t memorySize() const = 0;

    bool getColor() const { return isColored; }
};

class Point : public GraphObject {
    double x, y;
public:
    Point(double x = 0, double y = 0, bool colored = true)
        : GraphObject(colored), x(x), y(y) {}
    GraphObject* clone() const override { return new Point(*this); }
    void draw() const override {
        std::cout << (isColored ? "Color" : "B/W") << " Point (" << x << ", " << y << ")\n";
    }
    size_t memorySize() const override { return sizeof(Point); }
};

class Line : public GraphObject {
    double x1, y1, x2, y2;
public:
    Line(double x1=0, double y1=0, double x2=0, double y2=0, bool colored=true)
        : GraphObject(colored), x1(x1), y1(y1), x2(x2), y2(y2) {}
    GraphObject* clone() const override { return new Line(*this); }
    void draw() const override {
        std::cout << (isColored ? "Color" : "B/W") << " Line (" << x1 << "," << y1
                  << ")-(" << x2 << "," << y2 << ")\n";
    }
    size_t memorySize() const override { return sizeof(Line); }
};

class Circle : public GraphObject {
    double cx, cy, r;
public:
    Circle(double cx=0, double cy=0, double r=1, bool colored=true)
        : GraphObject(colored), cx(cx), cy(cy), r(r) {}
    GraphObject* clone() const override { return new Circle(*this); }
    void draw() const override {
        std::cout << (isColored ? "Color" : "B/W") << " Circle (" << cx << "," << cy << ") r=" << r << "\n";
    }
    size_t memorySize() const override { return sizeof(Circle); }
};

// ====================== 2. Singleton ======================
class Scene {
private:
    static Scene* instance;
    std::vector<GraphObject*> objects;
    Scene() = default;
public:
    static Scene* getInstance() {
        if (!instance) instance = new Scene();
        return instance;
    }
    void addObject(GraphObject* obj) {
        if (obj) objects.push_back(obj);
    }
    void drawAll() const {
        std::cout << "=== What the scene contains ===\n";
        for (const auto* obj : objects) obj->draw();
        std::cout << "========================\n\n";
    }
    void clear() {
        for (auto* obj : objects) delete obj;
        objects.clear();
    }
    ~Scene() { clear(); }
};
Scene* Scene::instance = nullptr;

// ====================== 3. Abstract Factory ======================
class AbstractGraphFactory {
public:
    virtual ~AbstractGraphFactory() = default;
    virtual GraphObject* createPoint(double x = 0, double y = 0) = 0;
    virtual GraphObject* createLine(double x1=0, double y1=0, double x2=0, double y2=0) = 0;
    virtual GraphObject* createCircle(double cx=0, double cy=0, double r=1) = 0;
};

class ColorGraphFactory : public AbstractGraphFactory {
public:
    GraphObject* createPoint(double x = 0, double y = 0) override {
        auto* p = new Point(x, y, true); Scene::getInstance()->addObject(p); return p;
    }
    GraphObject* createLine(double x1=0, double y1=0, double x2=0, double y2=0) override {
        auto* l = new Line(x1,y1,x2,y2,true); Scene::getInstance()->addObject(l); return l;
    }
    GraphObject* createCircle(double cx=0, double cy=0, double r=1) override {
        auto* c = new Circle(cx,cy,r,true); Scene::getInstance()->addObject(c); return c;
    }
};

// ====================== 4. Making Adapter(Wrapper) ======================
class ThirdPartyTriangle {
    double x1,y1,x2,y2,x3,y3;
public:
    ThirdPartyTriangle(double a1=0,double b1=0,double a2=0,double b2=0,double a3=0,double b3=0)
        : x1(a1),y1(b1),x2(a2),y2(b2),x3(a3),y3(b3) {}
    void render() const {
        std::cout << "Third-Party Triangle (" << x1 << "," << y1 << ") (" << x2 << "," << y2
                  << ") (" << x3 << "," << y3 << ")\n";
    }
};

class TriangleAdapter : public GraphObject {
    ThirdPartyTriangle* triangle;
public:
    TriangleAdapter(double x1=0, double y1=0, double x2=0, double y2=0, double x3=0, double y3=0, bool colored=true)
        : GraphObject(colored) {
        triangle = new ThirdPartyTriangle(x1,y1,x2,y2,x3,y3);
    }
    ~TriangleAdapter() override { delete triangle; }
    GraphObject* clone() const override { return new TriangleAdapter(*this); }
    void draw() const override {
        std::cout << (isColored ? "Color" : "B/W") << " ";
        triangle->render();
    }
    size_t memorySize() const override { return sizeof(TriangleAdapter); }
};

// ====================== 5. Making Composite ======================
class Composite : public GraphObject {
    std::vector<GraphObject*> children;
public:
    Composite(bool colored = true) : GraphObject(colored) {}
    ~Composite() override {
        for (auto* child : children) delete child;
    }
    void add(GraphObject* g) { if (g) children.push_back(g); }
    GraphObject* clone() const override {
        auto* copy = new Composite(isColored);
        for (auto* child : children) copy->add(child->clone());
        return copy;
    }
    void draw() const override {
        std::cout << "Composite (contains " << children.size() << " elements):\n";
        for (auto* child : children) child->draw();
    }
    size_t memorySize() const override { return sizeof(Composite); }
};

// ====================== 6. Making Decorator (triangle coloring) ======================
class FilledDecorator : public GraphObject {
    GraphObject* component;
public:
    FilledDecorator(GraphObject* c)
        : GraphObject(c->getColor()), component(c) {}
    ~FilledDecorator() override { delete component; }
    GraphObject* clone() const override { return new FilledDecorator(component->clone()); }
    void draw() const override {
        component->draw();
        std::cout << "   >>> This graphic object is filled! <<<\n";
    }
    size_t memorySize() const override { return sizeof(FilledDecorator); }
};

// ====================== 7. Making Facade ======================
class GraphicsFacade {
    AbstractGraphFactory* factory;
public:
    GraphicsFacade(AbstractGraphFactory* f) : factory(f) {}

    void buildSceneFromString(const std::string& command) {
        Scene::getInstance()->clear();
        std::istringstream iss(command);
        std::string token;
        GraphObject* pendingTriangle = nullptr;

        while (std::getline(iss, token, ';')) {
            token.erase(0, token.find_first_not_of(" \t"));
            if (token.empty()) continue;

            std::istringstream tss(token);
            char type;
            tss >> type;

            if (type == 'P' || type == 'p') {       // Point
                double x, y; char comma;
                tss >> x >> comma >> y;
                factory->createPoint(x, y);
            }
            else if (type == 'C' || type == 'c') {  // Circle
                double cx, cy, r; char c1, c2;
                tss >> cx >> c1 >> cy >> c2 >> r;
                factory->createCircle(cx, cy, r);
            }
            else if (type == 'T' || type == 't') {  // Triangle
                double x1,y1,x2,y2,x3,y3; char c1,c2,c3,c4,c5;
                tss >> x1 >> c1 >> y1 >> c2 >> x2 >> c3 >> y2 >> c4 >> x3 >> c5 >> y3;
                pendingTriangle = new TriangleAdapter(x1,y1,x2,y2,x3,y3,true);
            }
            else if (type == 'F' || type == 'f') {  // Color filling for the triangle
                if (pendingTriangle) {
                    GraphObject* filled = new FilledDecorator(pendingTriangle);
                    Scene::getInstance()->addObject(filled);
                    pendingTriangle = nullptr;
                }
            }
        }

        // If there is no F — regular triangle
        if (pendingTriangle) {
            Scene::getInstance()->addObject(pendingTriangle);
        }
    }
};

// ====================== main ======================
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ColorGraphFactory colorFactory;
    GraphicsFacade facade(&colorFactory);

    // Facade demonstration
    std::string command = "P 10,20; C 50,50,25; T 0,0,100,0,50,80; F";
    std::cout << "Facade query-string: " << command << "\n\n";
    facade.buildSceneFromString(command);
    Scene::getInstance()->drawAll();

    // === Composite demonstration ===
    std::cout << "=== Composite demonstration ===\n";
    Composite* group = new Composite();
    group->add(new Point(1,1,true));
    group->add(new Circle(5,5,10,true));
    group->draw();
    delete group;

    return a.exec();
}
