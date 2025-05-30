#include "SFML/Graphics.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <cstdint>
// #include <cstdlib>
#define 𝜏 6.283185307179586
#define π 3.141592653589793
sf::RenderWindow mainWindow;
float randomNum()
{
    return (static_cast<float>(rand()) / RAND_MAX);
    srand(rand());
}

#

class particle
{
private:
    sf::CircleShape circle;
    float mass;
    sf::Vector2f velocity;

private:
    inline float dot(const sf::Vector2f &V1, const sf::Vector2f &V2)
    {
        return V1.x * V2.x + V1.y * V2.y;
    }
    inline float distanceBetween(const sf::Vector2f &V1, const sf::Vector2f &V2)
    {
        return std::sqrt(std::pow(V1.x - V2.x, 2) + std::pow(V1.y - V2.y, 2));
    }
    inline float distance(const sf::Vector2f &V)
    {
        return std::sqrt(std::pow(V.x, 2) + std::pow(V.y, 2));
    }

public:
    particle(const float &x, const float &y, const float &radius_param, const sf::Vector2f &velocity_param, float mass_param = 0) : velocity(velocity_param)
    {
        circle.setRadius(radius_param);
        circle.setOrigin({radius_param, radius_param});
        circle.setPointCount(floorf(std::sqrt(radius_param) * 10) + 3);
        circle.setPosition({x, y});
        circle.setFillColor(sf::Color::White);
        mass = (mass_param == 0.0f) ? π * std::pow(radius_param, 2) : mass_param;
    }
    ~particle() {}

public:
    float getRadius() { return circle.getRadius(); }
    float getX() { return circle.getPosition().x; }
    float getY() { return circle.getPosition().y; }
    double getKE(void) { return 0.5 * this->mass * std::pow(distance(this->velocity), 2); }
    void setColor(sf::Color color_param) { this->circle.setFillColor(color_param); }

public:
    float distMouse()
    {
        return distanceBetween(this->circle.getPosition(), static_cast<sf::Vector2f>(sf::Mouse::getPosition(mainWindow)));
    }
    void bodyCollision(particle &other)
    {
        // Body Collision Detection and Resolution
        float dist = distanceBetween(this->circle.getPosition(), other.circle.getPosition());
        if (dist < this->circle.getRadius() + other.circle.getRadius())
        {
            float overlap = this->circle.getRadius() + other.circle.getRadius() - dist;
            sf::Vector2f n_hat = (other.circle.getPosition() - this->circle.getPosition()) / dist;
            this->circle.move(n_hat * (overlap / -2.0f));
            other.circle.move(n_hat * (overlap / 2.0f));
            float total_mass = this->mass + other.mass;
            sf::Vector2f velocity_differnece = other.velocity - this->velocity, position_differnece = other.circle.getPosition() - this->circle.getPosition();
            this->velocity += static_cast<float>(2 * (other.mass / total_mass) * (dot(velocity_differnece, position_differnece) / std::pow(distance(position_differnece), 2))) * position_differnece;
            other.velocity += static_cast<float>(2 * (this->mass / total_mass) * (dot(-velocity_differnece, -position_differnece) / std::pow(distance(-position_differnece), 2))) * -position_differnece;
        }
    }
    void edgeCollision()
    {
        // Edge Collision Detection and Resolution
        if (circle.getPosition().x - circle.getRadius() < 0)
        {
            velocity.x *= -1;
            this->circle.move({-2 * (circle.getPosition().x - circle.getRadius()), 0});
        }
        else if (circle.getPosition().x + circle.getRadius() > mainWindow.getSize().x)
        {
            velocity.x *= -1;
            this->circle.move({-2 * ((circle.getPosition().x + circle.getRadius()) - mainWindow.getSize().x), 0});
        }
        if (circle.getPosition().y - circle.getRadius() < 0)
        {
            velocity.y *= -1;
            this->circle.move({0, -2 * (circle.getPosition().y - circle.getRadius())});
        }
        else if (circle.getPosition().y + circle.getRadius() > mainWindow.getSize().y)
        {
            velocity.y *= -1;
            this->circle.move({0, -2 * ((circle.getPosition().y + circle.getRadius()) - mainWindow.getSize().y)});
        }
    }
    void update() { circle.move(velocity); }
    void display() { mainWindow.draw(circle); }
};

#

struct boundary
{
public:
    float x, y, halfWidth, halfHeight;

public:
    boundary(float x_param, float y_param, float halfWidth_param, float halfHeight_param)
        : x(x_param), y(y_param), halfWidth(halfWidth_param), halfHeight(halfHeight_param) {}
    ~boundary() {}

public:
    bool contains(particle &point_param)
    {
        return x - halfWidth <= point_param.getX() && point_param.getX() < x + halfWidth &&
               y - halfHeight <= point_param.getY() && point_param.getY() < y + halfHeight;
        /*
            Left boundary edge and top boundary edge are inclusive
            Right boundary edge and bottom boundary edge are exclusive
        */
    }
    bool intersects(const boundary &param)
    {
        return !(this->x + this->halfWidth < param.x - param.halfWidth ||
                 this->x - this->halfWidth > param.x + param.halfWidth ||
                 this->y + this->halfHeight < param.y - param.halfHeight ||
                 this->y - this->halfHeight > param.y + param.halfHeight);

        /*
            To do intersection test between two boundaries A and B we check
            Rigth edge of A is < Left edge of B
            Left edge of A is > Right edge of B
            Bottom edge of A is < Top edge of B
            Top edge of A is > Bottom edge of B
            if any of these is true it means they don't intersect
        */
    }
};

#

class node
{
private:
    std::vector<particle *> pointerList;
    boundary nodeBoundary;
    const unsigned short int capacity, maxSubdivisions;
    unsigned short int subdivisionLevel;
    bool isDivided;
    node *topRight, *topLeft, *bottomRight, *bottomLeft;

private:
    bool isLeaf() const { return !(this->isDivided); }
    void moveToLeaves()
    {
        for (particle *i : pointerList)
        {
            if (nodeBoundary.y <= i->getY())
            {
                if (nodeBoundary.x <= i->getX())
                {
                    bottomRight->pointerList.push_back(i);
                    if ((bottomRight->pointerList.size() > capacity) && (this->subdivisionLevel + 1 < maxSubdivisions))
                    {
                        bottomRight->subdivide();
                    }
                }
                else
                {
                    bottomLeft->pointerList.push_back(i);
                    if ((bottomLeft->pointerList.size() > capacity) && (this->subdivisionLevel + 1 < maxSubdivisions))
                    {
                        bottomLeft->subdivide();
                    }
                }
            }
            else
            {
                if (nodeBoundary.x <= i->getX())
                {
                    topRight->pointerList.push_back(i);
                    if ((topRight->pointerList.size() > capacity) && (this->subdivisionLevel + 1 < maxSubdivisions))
                    {
                        topRight->subdivide();
                    }
                }
                else
                {
                    topLeft->pointerList.push_back(i);
                    if ((topLeft->pointerList.size() > capacity) && (this->subdivisionLevel + 1 < maxSubdivisions))
                    {
                        topLeft->subdivide();
                    }
                }
            }
        }
    };
    void subdivide()
    {
        isDivided = true;
        float newHalfWidth = nodeBoundary.halfWidth / 2.0f, newHalfHeight = nodeBoundary.halfHeight / 2.0f;
        topRight = new node(boundary(nodeBoundary.x + newHalfWidth, nodeBoundary.y - newHalfHeight, newHalfWidth, newHalfHeight), capacity, maxSubdivisions, 1 + subdivisionLevel);
        topLeft = new node(boundary(nodeBoundary.x - newHalfWidth, nodeBoundary.y - newHalfHeight, newHalfWidth, newHalfHeight), capacity, maxSubdivisions, 1 + subdivisionLevel);
        bottomRight = new node(boundary(nodeBoundary.x + newHalfWidth, nodeBoundary.y + newHalfHeight, newHalfWidth, newHalfHeight), capacity, maxSubdivisions, 1 + subdivisionLevel);
        bottomLeft = new node(boundary(nodeBoundary.x - newHalfWidth, nodeBoundary.y + newHalfHeight, newHalfWidth, newHalfHeight), capacity, maxSubdivisions, 1 + subdivisionLevel);
        this->moveToLeaves();
        std::vector<particle *>{}.swap(pointerList);
    }

public:
    node(boundary boundary_param, unsigned int capacity_param, unsigned short int maxSubdivisions_param, unsigned short int subdivisionLevel_param = 0)
        : isDivided(false), topRight(nullptr), topLeft(nullptr), bottomRight(nullptr), bottomLeft(nullptr),
          nodeBoundary(boundary_param), capacity(capacity_param), maxSubdivisions(maxSubdivisions_param), subdivisionLevel(subdivisionLevel_param)
    {
        pointerList.reserve(capacity_param + 1);
    }
    ~node()
    {
        if (isDivided)
        {
            delete topRight;
            delete topLeft;
            delete bottomRight;
            delete bottomLeft;
        }
    }

public:
    void nestPoint(particle &param)
    {
        if (this->isLeaf())
        {
            pointerList.push_back(&param);
            if ((pointerList.size() > capacity) && (this->subdivisionLevel < maxSubdivisions))
            {
                this->subdivide();
            }
        }
        else
        {
            if (nodeBoundary.y <= param.getY())
            {
                if (nodeBoundary.x <= param.getX())
                {
                    bottomRight->nestPoint(param);
                }
                else
                {
                    bottomLeft->nestPoint(param);
                }
            }
            else
            {
                if (nodeBoundary.x <= param.getX())
                {
                    topRight->nestPoint(param);
                }
                else
                {
                    topLeft->nestPoint(param);
                }
            }
        }
    }
    void query(boundary range_param, std::vector<particle *> &container_param)
    {
        if (nodeBoundary.intersects(range_param))
        {
            if (this->isLeaf())
            {
                for (particle *i : this->pointerList)
                {
                    if (range_param.contains(*i))
                    {
                        container_param.push_back(i);
                    }
                }
            }
            else
            {
                topLeft->query(range_param, container_param);
                topRight->query(range_param, container_param);
                bottomRight->query(range_param, container_param);
                bottomLeft->query(range_param, container_param);
            }
        }
    }
    void drawTree()
    {
        sf::RectangleShape rectangle(sf::Vector2f(nodeBoundary.halfWidth * 2, nodeBoundary.halfHeight * 2));
        rectangle.setOutlineColor(sf::Color::Black);
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineThickness(1);
        rectangle.setPosition({nodeBoundary.x - nodeBoundary.halfWidth, nodeBoundary.y - nodeBoundary.halfHeight});
        mainWindow.draw(rectangle);
        if (this->isDivided)
        {
            topRight->drawTree();
            topLeft->drawTree();
            bottomRight->drawTree();
            bottomLeft->drawTree();
        }
    }
    void clearTree()
    {
        this->pointerList.clear();
        if (this->isDivided)
        {
            topRight->clearTree();
            topLeft->clearTree();
            bottomRight->clearTree();
            bottomLeft->clearTree();
        }
    }
};

#

int main(void)
{
    uint16_t x_res = 1366, y_res = 768;
    mainWindow.create(sf::VideoMode({x_res, y_res}), "Elastic Collisions", sf::Style::Titlebar|sf::Style::Close); // (1366, 768) (1920, 1080)
    mainWindow.setFramerateLimit(60);
    mainWindow.setKeyRepeatEnabled(false);
    sf::Clock Timer;
    std::vector<particle> particles;
    std::vector<particle *> bucket;
    node quadTree(boundary(static_cast<float>(x_res) / 2, static_cast<float>(y_res) / 2, static_cast<float>(x_res) / 2, static_cast<float>(y_res) / 2), 8, 5);
    bool isMBDown[3]{},isKKDown[sf::Keyboard::ScancodeCount]{};
    enum Mouse_Button:uint8_t{LMB, RMB, MMB};
    enum Key_Control:uint8_t
    {
        Red=static_cast<uint8_t>(sf::Keyboard::Scan::R),
        Orange=static_cast<uint8_t>(sf::Keyboard::Scan::O),
        Yellow=static_cast<uint8_t>(sf::Keyboard::Scan::Y),
        Green=static_cast<uint8_t>(sf::Keyboard::Scan::G),
        Blue=static_cast<uint8_t>(sf::Keyboard::Scan::B),
        Purple=static_cast<uint8_t>(sf::Keyboard::Scan::P),
        Violet=static_cast<uint8_t>(sf::Keyboard::Scan::V),
        White=static_cast<uint8_t>(sf::Keyboard::Scan::W),
        DeleteAll=static_cast<uint8_t>(sf::Keyboard::Scan::X),
        DeleteLast=static_cast<uint8_t>(sf::Keyboard::Scan::Backspace),
        quadTreeToggle=static_cast<uint8_t>(sf::Keyboard::Scan::Q),
        SystemEnergy=static_cast<uint8_t>(sf::Keyboard::Scan::K),
        FrameTime=static_cast<uint8_t>(sf::Keyboard::Scan::S)
    };
    while (mainWindow.isOpen())
    {
        Timer.restart();
        while (const std::optional event = mainWindow.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                mainWindow.close();
            else if (const auto *btn = event->getIf<sf::Event::MouseButtonPressed>())
            {
                isMBDown[static_cast<uint8_t>(btn->button)] = true;
                switch (static_cast<uint8_t>(btn->button))
                {
                case LMB:
                    std::clog<<"LMB"<<std::endl;
                    break;
                case RMB:
                    std::clog<<"RMB"<<std::endl;
                    break;
                case MMB:
                    std::clog<<"MMB"<<std::endl;
                    break;
                }
            }
            else if (const auto *btn = event->getIf<sf::Event::MouseButtonReleased>())
                isMBDown[static_cast<unsigned short int>(btn->button)] = false;
            // else if (const auto *resize = event->getIf<sf::Event::Resized>())
            //     std::clog << "Main Window Resized now " << resize->size.x << " by " << resize->size.y << " pixels\n";
            else if (event->is<sf::Event::FocusLost>())
                std::clog << "Main Window Unfocussed\n";
            else if (event->is<sf::Event::FocusGained>())
                std::clog << "Main Window Refocussed\n";
            else if (const auto *key = event->getIf<sf::Event::TextEntered>())
            {
                if (31 < key->unicode && key->unicode < 128)
                    std::clog << static_cast<unsigned char>(key->unicode) << std::endl;
            }
            else if (const auto *key = event->getIf<sf::Event::KeyPressed>())
            {
                isKKDown[static_cast<uint8_t>(key->scancode)]=true;
                if (key->scancode==sf::Keyboard::Scancode::Escape)
                    mainWindow.close();
            }
            else if (const auto *key = event->getIf<sf::Event::KeyReleased>())
                isKKDown[static_cast<uint8_t>(key->scancode)]=false;
            
        }
        if (mainWindow.hasFocus())
        {
            //note: Deleting or Adding elements in particles vector makes QuadTree pointer list useless 
            if (isKKDown[DeleteAll]) particles.clear();
            else if (isKKDown[DeleteLast])
            {
                if (particles.size() > 0)
                    particles.erase(begin(particles));
            }
            else if (isMBDown[LMB])
            {
                float randNum = randomNum();
                particles.push_back(particle(sf::Mouse::getPosition(mainWindow).x, sf::Mouse::getPosition(mainWindow).y, randomNum() * 2.0f + 5.0f, sf::Vector2f(std::cos(randNum * 𝜏), std::sin(randNum * 𝜏))*(randomNum()+0.05f)));
                if (isKKDown[Red])
                    particles.back().setColor(sf::Color(156, 56, 72, 255));
                else if (isKKDown[Orange])
                    particles.back().setColor(sf::Color(209, 96, 61, 255));
                else if (isKKDown[Yellow])
                    particles.back().setColor(sf::Color(254, 185, 95, 255));
                else if (isKKDown[Green])
                    particles.back().setColor(sf::Color(122, 199, 79, 255));
                else if (isKKDown[Blue])
                    particles.back().setColor(sf::Color(60, 145, 230, 255));
                else if (isKKDown[Purple])
                    particles.back().setColor(sf::Color(188, 150, 230, 255));
                else if (isKKDown[Violet])
                    particles.back().setColor(sf::Color(47, 0, 79, 255));
            }
            // # Calculate and Display
            mainWindow.clear(sf::Color(25, 30, 40, 225));
            /* Old Method          note: Good for under 500 particles
            for (size_t i = 0; i < particles.size(); ++i)
            {
                for (size_t j = i + 1; j < particles.size(); ++j) // If you collide first thing with second it also means you collided second thing with first 
                {
                    particles[i].bodyCollision(particles[j]);
                }
            }
            */
            // QuadTree Method     note: Good for about 2000 particles
            for (particle &i : particles)
            {
                quadTree.nestPoint(i);
                // Method is placing pointers to particles in nodes they belong to
            }
            if (isKKDown[quadTreeToggle]) quadTree.drawTree();
            for (particle &i : particles)
            {
                quadTree.query(boundary(i.getX(), i.getY(), i.getRadius() + 10.0f, i.getRadius() + 10.0f), bucket); // Range boundary must be slightly more than radius of this ball + radius of largest possible ball
                // Retrieving pointers to particles those fall in contact range of this particle in a bucket
                for (particle *j : bucket)
                {
                    if (&i != j) // Make sure pointers aren't the same
                        i.bodyCollision(*j);
                }
                bucket.clear();
            }
            // # Coloring
            if(isMBDown[RMB])
            {
                quadTree.query(boundary(sf::Mouse::getPosition(mainWindow).x, sf::Mouse::getPosition(mainWindow).y, 20.0f,20.0f), bucket);
                for (particle *j : bucket)
                {
                    if (j->distMouse() < j->getRadius())
                    {
                        if (isKKDown[Yellow])
                            j->setColor(sf::Color(254, 185, 95, 255));
                        else if (isKKDown[Red])
                            j->setColor(sf::Color(156, 56, 72, 255));
                        else if (isKKDown[Blue])
                            j->setColor(sf::Color(60, 145, 230, 255));
                        else if (isKKDown[Green])
                            j->setColor(sf::Color(122, 199, 79, 255));
                        else if (isKKDown[Purple])
                            j->setColor(sf::Color(188, 150, 230, 255));
                        else if (isKKDown[Violet])
                            j->setColor(sf::Color(47, 0, 79, 255));
                        else if (isKKDown[Orange])
                            j->setColor(sf::Color(209, 96, 61, 255));
                        else if (isKKDown[White])
                            j->setColor(sf::Color(255, 255, 255, 255));
                        else
                            particles.erase(particles.begin() + (j - particles.data()));
                    }
                }
                bucket.clear();
            }
            quadTree.clearTree();
            for (particle &i : particles)
            {
                i.edgeCollision();
                i.update();
                i.display();
            }
            mainWindow.display();
            if (isKKDown[FrameTime]) std::clog << "Time:" << Timer.getElapsedTime().asMicroseconds()/1000000.0 << std::endl;
            else if(isKKDown[SystemEnergy])
            {
                std::cout << "Particles: " << particles.size() << "\n";
                double total_KE = 0;
                for (particle &i : particles){total_KE += i.getKE();}
                std::cout << "System Kinetic Energy: " << total_KE << "\n";
            }
        }
    }
    return 0;
}
/*
    Controls:
    Left Mouse Button       : Adds White particles
    Left Mouse Button + P   : Adds Purple particles
    Left Mouse Button + V   : Adds Voilet particles
    Left Mouse Button + B   : Adds Blue particles
    Left Mouse Button + G   : Adds Green particles
    Left Mouse Button + Y   : Adds Yellow particles
    Left Mouse Button + O   : Adds Orange particles
    Left Mouse Button + R   : Adds Red particles

    Right Mouse Button      : Deletes particle
    Right Mouse Button + P  : Colors particle Purple
    Right Mouse Button + V  : Colors particle Voilet
    Right Mouse Button + B  : Colors particle Blue
    Right Mouse Button + G  : Colors particle Green
    Right Mouse Button + Y  : Colors particle Yellow
    Right Mouse Button + O  : Colors particle Orange
    Right Mouse Button + R  : Colors particle Red
    Right Mouse Button + W  : Colors particle White

    Back Space Button       : Deletes old particles
    X                       : Deletes all particles
    Q                       : Toggle Quad Tree visibility
    K                       : Info
    S                       : Tells how much time frame took in seconds
    Esc                     : Exits Program

    Last Edited on 30th May 2025
*/