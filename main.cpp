#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <list>
#include "include/imgui.h"
#include "include/imgui-SFML.h"
#include "Chronometer.hpp"

#define PI 3.14159265

struct Engine {
    float mass;
    float k;
    float omega;
    float Xmax;
    float period;
    float phi;
    sftools::Chronometer clock;
    float graphSpeed;
};

struct Graphic {
    std::vector<sf::RectangleShape*> drawables;
    std::list<sf::CircleShape*> graph;
    std::vector<sf::Text*> hud;
    sf::Font cascadia;
};

float pos(struct Engine* e);
float x(struct Engine* e);
float vel(struct Engine* e);
float acc(struct Engine* e);
float calcOmega(struct Engine* e);
float calcPeriod(struct Engine* e);
void setPeriod(struct Engine* e, struct Graphic* g, float period);
void initEngine(struct Engine* e);
void initSpring(struct Graphic* g);
void initAxis(struct Graphic* g);
void initHud(struct Engine* e, struct Graphic* g);
void shiftGraph(struct Engine* e, struct Graphic* g);
void graphPoint(struct Graphic* g, float y);
void updateValues(struct Engine* e, struct Graphic* g);
void render(sf::RenderWindow* window, struct Graphic* g);

int main() {
    double aspect_ratio = 16.0 / 9;
    int width = 1280;
    int height = int(width / aspect_ratio);
    sf::RenderWindow window(sf::VideoMode(width, height), "Simple Harmonic Motion");

    ImGui::SFML::Init(window);
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    
    struct Engine e;
    struct Graphic g;

    initEngine(&e);
    initSpring(&g);
    initAxis(&g);
    initHud(&e, &g);

    double dt = 1.f/60.f; // Modify this to change physics rate.
    double accumulator = 0.f;
    sf::Clock clock;
    sf::Clock clockImGui;
    sf::Clock fpsClk;
    unsigned int fps = 0;

    bool pause = true;
    float period = e.period;
    float omega = e.omega;
    float mass = e.mass;
    float k = e.k;
    float f = e.omega / (2 * PI);
    float simTime = e.clock.getElapsedTime().asSeconds();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, clockImGui.restart());
        ImGui::Begin("options", NULL, window_flags);
        ImGui::Checkbox("pause", &pause);
        ImGui::Text("<- -             + ->");
        ImGui::DragFloat("x max", &e.Xmax, 0.1f, 0.f, 1000.f); // v
        ImGui::DragFloat("w", &omega, 0.1f, 0.f, 1000.f); // v
        ImGui::DragFloat("mass", &mass, 0.1f, 0.f, 1000.f); //
        ImGui::DragFloat("k", &k, 0.1f, 0.f, 1000000.f);
        ImGui::DragFloat("f", &f, 0.1f, 0.f, 1000.f);
        ImGui::DragFloat("phi", &e.phi, 0.1f, - 2 * PI, 2 * PI);
        ImGui::DragFloat("period", &period, 0.1f, 0.f, 1000.f);
        ImGui::DragFloat("time", &simTime, 0.1f, 0.f, 1000.f);
        ImGui::End();
        ImGui::EndFrame();

        accumulator += clock.getElapsedTime().asMicroseconds() / 1000000.f;
        clock.restart();
        if (accumulator >= dt) {
            // Physics and stuff
            if (period != e.period) {
                setPeriod(&e, &g, period);
                omega = e.omega;
                k = e.k;
                f = 1 / period;
            } else if (omega != e.omega) {
                setPeriod(&e, &g, 2 * PI / omega);
                k = e.k;
                period = e.period;
                f = 1 / period;
            } else if (mass != e.mass) {
                if (mass) {
                    e.mass = mass;
                    setPeriod(&e, &g, 2 * PI / calcOmega(&e));
                    omega = e.omega;
                    period = e.period;
                    f = 1 / period;
                } else { pause = true; }
            } else if (k != e.k) {
                e.k = k;
                setPeriod(&e, &g, 2 * PI / calcOmega(&e));
                omega = e.omega;
                period = e.period;
                f = 1 / e.period;
            } else if ( 1 / f != e.period) {
                setPeriod(&e, &g, 1 / f);
                omega = e.omega;
                period = e.period;
                k = e.k;
            } else if (!e.clock.isRunning()) {
                float curTime = e.clock.getElapsedTime().asSeconds();
                if (simTime != curTime) {
                    e.clock.add(sf::seconds(simTime - curTime));
                }
            }
            simTime = e.clock.getElapsedTime().asSeconds();

            updateValues(&e, &g);
            g.drawables[1]->setSize(sf::Vector2f(g.drawables[1]->getSize().x, 216 - pos(&e)));
            g.drawables[2]->setPosition(g.drawables[2]->getPosition().x, 360 - pos(&e));
            g.drawables[9]->setPosition(g.drawables[9]->getPosition().x, 362 - pos(&e));

            if (!pause) {
                e.clock.resume();
                if (e.omega != 0) {
                    graphPoint(&g, g.drawables[2]->getPosition().y);
                    shiftGraph(&e, &g);
                }
            }
            else { e.clock.pause(); }

            accumulator = 0;

            fps++;
            render(&window, &g);
            ImGui::SFML::Render(window);
            window.display();
        }

        if (fpsClk.getElapsedTime().asMilliseconds() >= 1000) {
            fpsClk.restart();
            std::cout << fps << std::endl;
            fps = 0;
        }
    }
    ImGui::SFML::Shutdown();

    return 0;
}

float pos(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return (((e->Xmax > 4) ? 4 : e->Xmax) * 50) * cosf(e->omega * t + e->phi);
}

float x(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return e->Xmax * cosf(e->omega * t + e->phi);
}

float vel(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return - e->omega * e->Xmax * sinf(e->omega * t + e->phi);
}

float acc(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return e->omega * e->omega * e->Xmax * cosf(e->omega * t + e->phi);
}

float calcOmega(struct Engine* e) {
    return sqrtf(e->k / e->mass);
}

float calcPeriod(struct Engine* e) {
    return 2 * PI * sqrtf(e->mass / e->k);
}

void setPeriod(struct Engine* e, struct Graphic* g, float period) {
    e->period = period;
    e->omega = 2 * PI / period;
    e->k = e->omega * e->omega * e->mass;
    e->graphSpeed = -5.f / e->period;
}

void initEngine(struct Engine* e) {
    e->k = 1;
    e->mass = 1;
    e->Xmax = 1;
    e->phi = 0;
    e->period = 1;
    e->omega = 2 * PI / e->period;
    e->k = e->omega * e->omega * e->mass;
    e->graphSpeed = -5;
}

void initSpring(struct Graphic* g) {
    sf::RectangleShape* ceiling = new sf::RectangleShape(sf::Vector2f(325, 23));
    ceiling->setOrigin(325/2-~(325&0x01), 23/2-~(23&0x01));
    ceiling->setFillColor(sf::Color(255, 247, 238, 220));
    ceiling->setPosition(997, 63);
    g->drawables.push_back(ceiling);

    sf::Texture* texture = new sf::Texture;
    texture->loadFromFile("spring.png");
    sf::RectangleShape* spring = new sf::RectangleShape(sf::Vector2f(94, 216)); // 481
    spring->setTexture(texture);
    spring->setOrigin(94/2-~(94&0x01), 0);
    spring->setPosition(997, 72);
    g->drawables.push_back(spring);

    sf::RectangleShape* box = new sf::RectangleShape(sf::Vector2f(134, 134));
    box->setFillColor(sf::Color(0, 0, 0));
    box->setOutlineThickness(5);
    box->setOutlineColor(sf::Color(255, 247, 238, 220));
    box->setOrigin(134/2-~(134&0x01), 134/2-~(134&0x01));
    box->setPosition(997, 360);
    g->drawables.push_back(box);
}

void initAxis(struct Graphic* g) {
    sf::RectangleShape* timeAxis = new sf::RectangleShape(sf::Vector2f(860, 5));
    timeAxis->setFillColor(sf::Color(220, 213, 205));
    timeAxis->setPosition(0, 720/2);
    g->drawables.push_back(timeAxis);
    sf::RectangleShape* upArrow = new sf::RectangleShape(sf::Vector2f(15, 5));
    upArrow->setFillColor(sf::Color(220, 213, 205));
    upArrow->setPosition(859, 364);
    upArrow->setOrigin(14, 4);
    upArrow->rotate(45);
    g->drawables.push_back(upArrow);
    sf::RectangleShape* dwnArrow = new sf::RectangleShape(sf::Vector2f(15, 5));
    dwnArrow->setFillColor(sf::Color(220, 213, 205));
    dwnArrow->setPosition(859, 360);
    dwnArrow->setOrigin(14, 0);
    dwnArrow->rotate(-45);
    g->drawables.push_back(dwnArrow);

    sf::RectangleShape* posAxis = new sf::RectangleShape(sf::Vector2f(5, 558));
    posAxis->setFillColor(sf::Color(220, 213, 205));
    posAxis->setPosition(800, 80);
    g->drawables.push_back(posAxis);
    sf::RectangleShape* lftArrow = new sf::RectangleShape(sf::Vector2f(5, 15));
    lftArrow->setFillColor(sf::Color(220, 213, 205));
    lftArrow->setPosition(803, 77);
    lftArrow->rotate(45);
    g->drawables.push_back(lftArrow);
    sf::RectangleShape* rgtArrow = new sf::RectangleShape(sf::Vector2f(5, 15));
    rgtArrow->setFillColor(sf::Color(220, 213, 205));
    rgtArrow->setPosition(799, 81);
    rgtArrow->rotate(-45);
    g->drawables.push_back(rgtArrow);

    sf::RectangleShape* bar = new sf::RectangleShape(sf::Vector2f(40, 3));
    bar->setFillColor(sf::Color(220, 213, 205));
    bar->setOrigin(19, 1);
    bar->setPosition(801, 362);
    g->drawables.push_back(bar);
}

void initHud(struct Engine* e, struct Graphic* g) {
    g->cascadia.loadFromFile("CascadiaCode-Regular.otf");

    sf::Text* bar = new sf::Text;
    bar->setFont(g->cascadia);
    bar->setFillColor(sf::Color::White);
    bar->setCharacterSize(20);
    bar->setPosition(860, 350);
    g->hud.push_back(bar);

    std::string s = std::to_string(e->Xmax);
    s = s.substr(0, s.find('.') + 3);

    sf::Text* Xm = new sf::Text;
    Xm->setFont(g->cascadia);
    Xm->setFillColor(sf::Color::White);
    Xm->setCharacterSize(20);
    Xm->setPosition(40, 620);
    Xm->setString("x max: " + s + " m");
    g->hud.push_back(Xm);

    s = std::to_string(e->omega);
    s = s.substr(0, s.find('.') + 3);

    sf::Text* omega = new sf::Text;
    omega->setFont(g->cascadia);
    omega->setFillColor(sf::Color::White);
    omega->setCharacterSize(20);
    omega->setPosition(40, 660);
    omega->setString("w: " + s + " rad/s");
    g->hud.push_back(omega);

    s = std::to_string(e->mass);
    s = s.substr(0, s.find('.') + 3);

    sf::Text* m = new sf::Text;
    m->setFont(g->cascadia);
    m->setFillColor(sf::Color::White);
    m->setCharacterSize(20);
    m->setPosition(240, 620);
    m->setString("m: " + s + " Kg");
    g->hud.push_back(m);

    s = std::to_string(e->k);
    s = s.substr(0, s.find('.') + 3);

    sf::Text* k = new sf::Text;
    k->setFont(g->cascadia);
    k->setFillColor(sf::Color::White);
    k->setCharacterSize(20);
    k->setPosition(240, 660);
    k->setString("k: " + s + " N/m");
    g->hud.push_back(k);

    s = std::to_string(e->omega / (2 * PI));
    s = s.substr(0, s.find('.') + 3);

    sf::Text* f = new sf::Text;
    f->setFont(g->cascadia);
    f->setFillColor(sf::Color::White);
    f->setCharacterSize(20);
    f->setPosition(420, 620);
    f->setString("f: " + s + " Hz");
    g->hud.push_back(f);

    s = std::to_string(e->phi);
    s = s.substr(0, s.find('.') + 3);

    sf::Text* phi = new sf::Text;
    phi->setFont(g->cascadia);
    phi->setFillColor(sf::Color::White);
    phi->setCharacterSize(20);
    phi->setPosition(420, 660);
    phi->setString("phi: " + s + " rad");
    g->hud.push_back(phi);

    s = std::to_string((2 * PI) / e->omega);
    s = s.substr(0, s.find('.') + 3);

    sf::Text* p = new sf::Text;
    p->setFont(g->cascadia);
    p->setFillColor(sf::Color::White);
    p->setCharacterSize(20);
    p->setPosition(630, 620);
    p->setString("T: " + s + " s");
    g->hud.push_back(p);

    sf::Text* clk = new sf::Text;
    clk->setFont(g->cascadia);
    clk->setFillColor(sf::Color::White);
    clk->setCharacterSize(20);
    clk->setPosition(630, 660);
    clk->setString("time:");
    g->hud.push_back(clk);

    sf::Text* xt = new sf::Text;
    xt->setFont(g->cascadia);
    xt->setFillColor(sf::Color::White);
    xt->setCharacterSize(20);
    xt->setPosition(40, 40);
    xt->setString("x(t):");
    g->hud.push_back(xt);

    sf::Text* vt = new sf::Text;
    vt->setFont(g->cascadia);
    vt->setFillColor(sf::Color::White);
    vt->setCharacterSize(20);
    vt->setPosition(300, 40);
    vt->setString("v(t):");
    g->hud.push_back(vt);

    sf::Text* at = new sf::Text;
    at->setFont(g->cascadia);
    at->setFillColor(sf::Color::White);
    at->setCharacterSize(20);
    at->setPosition(560, 40);
    at->setString("a(t):");
    g->hud.push_back(at);
}

void shiftGraph(struct Engine* e, struct Graphic* g) {
    int lim = g->graph.size();
    for (std::list<sf::CircleShape*>::iterator it = g->graph.begin(); it != g->graph.end(); it++) {
        (*it)->move(e->graphSpeed, 0); // -2
        if ((*it)->getPosition().x < -5) {
            delete *it;
            g->graph.erase(it);
        }
    }
}

void graphPoint(struct Graphic* g, float y) {
    sf::CircleShape* p = new sf::CircleShape(2.5);
    p->setFillColor(sf::Color(0, 148, 255));
    p->setPosition(800, y);
    g->graph.push_back(p);
}

void updateValues(struct Engine* e, struct Graphic* g) {
    std::string s = std::to_string(x(e));
    s = s.substr(0, s.find('.') + 3);
    g->hud[0]->setString(s);
    g->hud[0]->setPosition(860, 350 - pos(e));
    g->hud[9]->setString("x(t): " + s + " m");

    s = std::to_string(e->Xmax);
    s = s.substr(0, s.find('.') + 3);
    g->hud[1]->setString("x max: " + s + " m");

    s = std::to_string(e->omega);
    s = s.substr(0, s.find('.') + 3);
    g->hud[2]->setString("w: " + s + " rad/s");

    s = std::to_string(e->mass);
    s = s.substr(0, s.find('.') + 3);
    g->hud[3]->setString("m: " + s + " Kg");

    s = std::to_string(e->k);
    s = s.substr(0, s.find('.') + 3);
    g->hud[4]->setString("k: " + s + " N/m");

    s = std::to_string(e->omega / (2 * PI));
    s = s.substr(0, s.find('.') + 3);
    g->hud[5]->setString("f: " + s + " Hz");

    s = std::to_string(e->phi);
    s = s.substr(0, s.find('.') + 3);
    g->hud[6]->setString("phi: " + s + " rad");

    s = std::to_string((2 * PI) / e->omega);
    s = s.substr(0, s.find('.') + 3);
    g->hud[7]->setString("T: " + s + " s");

    unsigned int total = e->clock.getElapsedTime().asSeconds();
    unsigned int hr = total / 3600;
    total -= hr * 3600;
    unsigned int min = total / 60;
    total -= min * 60;
    unsigned int sec = total;

    s = "time: ";
    if (hr < 10) { s = s + "0"; }
    s = s + std::to_string(hr) + ":";
    if (min < 10) { s = s + "0"; }
    s = s + std::to_string(min) + ":";
    if (sec < 10) { s = s + "0"; }
    s = s + std::to_string(sec);
    g->hud[8]->setString(s);

    s = std::to_string(vel(e));
    s = s.substr(0, s.find('.') + 3);
    g->hud[10]->setString("v(t): " + s + " m/s");

    s = std::to_string(acc(e));
    s = s.substr(0, s.find('.') + 3);
    g->hud[11]->setString("a(t): " + s + " m/s^2");
}

void render(sf::RenderWindow* window, struct Graphic* g) {
    window->clear();

    int lim = g->drawables.size();
    for (int i = 0; i < lim; i++) {
        window->draw(*(g->drawables)[i]);
    }

    std::list<sf::CircleShape*>::iterator it2 = g->graph.begin();
    it2++;
    for (std::list<sf::CircleShape*>::iterator it = g->graph.begin(); it != g->graph.end(); it++, it2++) {
        if (it2 != g->graph.end()) {
            float dx = (*it2)->getPosition().x - (*it)->getPosition().x;
            float dy = (*it2)->getPosition().y - (*it)->getPosition().y;
            float dist = sqrtf((dx*dx) + (dy * dy));
            float angle = atan2f(dy,dx) * 180 / PI;
            sf::RectangleShape bridge(sf::Vector2f(dist, 5));
            bridge.setPosition((*it)->getPosition().x + 2.5, (*it)->getPosition().y+2.5);
            bridge.setFillColor(sf::Color(0, 148, 255));
            bridge.setOrigin(0, 2.5);
            bridge.setRotation(angle);
            window->draw(bridge);
        }
        window->draw(**it);
    }

    lim = g->hud.size();
    for (int i = 0; i < lim; i++) {
        window->draw(*(g->hud)[i]);
    }
}
