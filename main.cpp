#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <list>

#define PI 3.14159265

struct Engine {
    float mass;
    float k;
    float omega;
    float Xmax;
    float period;
    float phi;
    sf::Clock clock;
};


float pos(struct Engine* e);
float vel(struct Engine* e);
float acc(struct Engine* e);
float calcOmega(struct Engine* e);
float calcPeriod(struct Engine* e);
void setPeriod(struct Engine* e, float period);
void initEngine(struct Engine* e);
void initSpring(std::vector<sf::RectangleShape*>* drawables);
void initAxis(std::vector<sf::RectangleShape*>* drawables);
void shiftGraph(std::list<sf::CircleShape*>* graph);
void graphPoint(std::list<sf::CircleShape*>* graph, float y);
void render(sf::RenderWindow* window, std::vector<sf::RectangleShape*>* drawables, std::list<sf::CircleShape*>* graph);

int main() {
    double aspect_ratio = 16.0 / 9;
    int width = 1280;
    int height = int(width / aspect_ratio);
    sf::RenderWindow window(sf::VideoMode(width, height), "Simple Harmonic Motion");
    std::vector<sf::RectangleShape*> drawables;
    std::list<sf::CircleShape*> graph;
    struct Engine e;

    initEngine(&e);
    initSpring(&drawables);
    initAxis(&drawables);

    double dt = 1.f/60.f; // Modify this to change physics rate.
    double accumulator = 0.f;
    sf::Clock clock;
    sf::Clock fpsClk;
    bool drawn = false;
    unsigned int fps = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        accumulator += clock.getElapsedTime().asMicroseconds() / 1000000.f;
        clock.restart();
        if (accumulator >= dt) {
            // Physics and stuff

            //e.clock.restart();
            drawables[1]->setSize(sf::Vector2f(drawables[1]->getSize().x, 216 - pos(&e)));
            drawables[2]->setPosition(drawables[2]->getPosition().x, 360 - pos(&e));
            
            graphPoint(&graph, drawables[2]->getPosition().y);

            accumulator = 0;
            drawn = false;
        }

        if (!drawn) {
            fps++;
            render(&window, &drawables, &graph);
            shiftGraph(&graph);
            drawn = true;
        }

        if (fpsClk.getElapsedTime().asMilliseconds() >= 1000) {
            fpsClk.restart();
            std::cout << fps << std::endl;
            fps = 0;
        }
    }


    return 0;
}

float pos(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return (((e->Xmax > 4) ? 4 : e->Xmax) * 50) * cosf(e->omega * t + e->phi);
}

float vel(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return - e->omega * (((e->Xmax > 4) ? 4 : e->Xmax) * 50) * sinf(e->omega * t + e->phi);
}

float acc(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return e->omega * e->omega * (((e->Xmax > 4) ? 4 : e->Xmax) * 50) * cosf(e->omega * t + e->phi);
}

float calcOmega(struct Engine* e) {
    return sqrtf(e->k / e->mass);
}

float calcPeriod(struct Engine* e) {
    return 2 * PI * sqrtf(e->mass / e->k);
}

void setPeriod(struct Engine* e, float period) {
    e->period = period;
    e->omega = 2 * PI / period;
    e->k = e->omega * e->omega * e->mass;
}

void initEngine(struct Engine* e) {
    e->k = 1;
    e->mass = 1;
    e->Xmax = 1;
    e->phi = 0;
    setPeriod(e, 1);
}

void initSpring(std::vector<sf::RectangleShape*>* drawables) {
    sf::RectangleShape* ceiling = new sf::RectangleShape(sf::Vector2f(325, 23));
    ceiling->setOrigin(325/2-~(325&0x01), 23/2-~(23&0x01));
    ceiling->setFillColor(sf::Color(255, 247, 238, 220));
    ceiling->setPosition(997, 63);
    drawables->push_back(ceiling);

    sf::Texture* texture = new sf::Texture;
    texture->loadFromFile("spring.png");
    sf::RectangleShape* spring = new sf::RectangleShape(sf::Vector2f(94, 216)); // 481
    spring->setTexture(texture);
    spring->setOrigin(94/2-~(94&0x01), 0);
    spring->setPosition(997, 72);
    drawables->push_back(spring);

    sf::RectangleShape* box = new sf::RectangleShape(sf::Vector2f(134, 134));
    box->setFillColor(sf::Color(0, 0, 0));
    box->setOutlineThickness(5);
    box->setOutlineColor(sf::Color(255, 247, 238, 220));
    box->setOrigin(134/2-~(134&0x01), 134/2-~(134&0x01));
    box->setPosition(997, 360);
    drawables->push_back(box);
}

void initAxis(std::vector<sf::RectangleShape*>* drawables) {
    sf::RectangleShape* timeAxis = new sf::RectangleShape(sf::Vector2f(860, 5));
    timeAxis->setFillColor(sf::Color(220, 213, 205));
    timeAxis->setPosition(0, 720/2);
    drawables->push_back(timeAxis);
    sf::RectangleShape* upArrow = new sf::RectangleShape(sf::Vector2f(15, 5));
    upArrow->setFillColor(sf::Color(220, 213, 205));
    upArrow->setPosition(859, 364);
    upArrow->setOrigin(14, 4);
    upArrow->rotate(45);
    drawables->push_back(upArrow);
    sf::RectangleShape* dwnArrow = new sf::RectangleShape(sf::Vector2f(15, 5));
    dwnArrow->setFillColor(sf::Color(220, 213, 205));
    dwnArrow->setPosition(859, 360);
    dwnArrow->setOrigin(14, 0);
    dwnArrow->rotate(-45);
    drawables->push_back(dwnArrow);

    sf::RectangleShape* posAxis = new sf::RectangleShape(sf::Vector2f(5, 558));
    posAxis->setFillColor(sf::Color(220, 213, 205));
    posAxis->setPosition(800, 80);
    drawables->push_back(posAxis);
    sf::RectangleShape* lftArrow = new sf::RectangleShape(sf::Vector2f(5, 15));
    lftArrow->setFillColor(sf::Color(220, 213, 205));
    lftArrow->setPosition(803, 77);
    lftArrow->rotate(45);
    drawables->push_back(lftArrow);
    sf::RectangleShape* rgtArrow = new sf::RectangleShape(sf::Vector2f(5, 15));
    rgtArrow->setFillColor(sf::Color(220, 213, 205));
    rgtArrow->setPosition(799, 81);
    rgtArrow->rotate(-45);
    drawables->push_back(rgtArrow);
}

void shiftGraph(std::list<sf::CircleShape*>* graph) {
    int lim = graph->size();
    for (std::list<sf::CircleShape*>::iterator it = graph->begin(); it != graph->end(); it++) {
        (*it)->move(-2, 0);
        if ((*it)->getPosition().x < -5) {
            delete *it;
            graph->erase(it);
        }
    }
}

void graphPoint(std::list<sf::CircleShape*>* graph, float y) {
    sf::CircleShape* p = new sf::CircleShape(2.5);
    p->setFillColor(sf::Color(0, 148, 255));
    p->setPosition(800, y);
    graph->push_back(p);
}

void render(sf::RenderWindow* window, std::vector<sf::RectangleShape*>* drawables, std::list<sf::CircleShape*>* graph) {
    window->clear();

    int lim = drawables->size();
    for (int i = 0; i < lim; i++) {
        window->draw(*(*drawables)[i]);
    }

    //TODO: segmento de reta entre it e it+1
    std::list<sf::CircleShape*>::iterator it2 = graph->begin();
    it2++;
    for (std::list<sf::CircleShape*>::iterator it = graph->begin(); it != graph->end(); it++, it2++) {
        if (it2 == graph->end()) { continue; }

        window->draw(**it);
        float dx = (*it2)->getPosition().x - (*it)->getPosition().x;
        float dy = (*it2)->getPosition().y - (*it)->getPosition().y;
        float dist = sqrtf((dx*dx) + (dy * dy));
        float angle = atan2f(dy,dx) * 180 / PI;

        sf::RectangleShape bridge(sf::Vector2f(dist, 5));
        bridge.setPosition((*it)->getPosition().x + 2.5, (*it)->getPosition().y);
        bridge.setFillColor(sf::Color(0, 148, 255));
        bridge.setOrigin(0, 2.5);
        bridge.setRotation(angle);
        window->draw(bridge);
    }

    window->display();
}
