#include "SFML/Graphics.hpp"
#include <cmath>
#include <vector>
#include <iostream>
// #include <cstdlib>
// #include <cstdint>
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
        circle.setOrigin(radius_param, radius_param);
        circle.setPointCount(floorf(std::sqrt(radius_param) * 10) + 3);
        circle.setPosition(x, y);
        circle.setFillColor(sf::Color::White);
        mass = (mass_param == 0) ? π * std::pow(radius_param, 2) : mass_param;
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
            this->circle.move(-2 * (circle.getPosition().x - circle.getRadius()), 0);
        }
        else if (circle.getPosition().x + circle.getRadius() > mainWindow.getSize().x)
        {
            velocity.x *= -1;
            this->circle.move(-2 * ((circle.getPosition().x + circle.getRadius()) - mainWindow.getSize().x), 0);
        }
        if (circle.getPosition().y - circle.getRadius() < 0)
        {
            velocity.y *= -1;
            this->circle.move(0, -2 * (circle.getPosition().y - circle.getRadius()));
        }
        else if (circle.getPosition().y + circle.getRadius() > mainWindow.getSize().y)
        {
            velocity.y *= -1;
            this->circle.move(0, -2 * ((circle.getPosition().y + circle.getRadius()) - mainWindow.getSize().y));
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
        rectangle.setPosition(nodeBoundary.x - nodeBoundary.halfWidth, nodeBoundary.y - nodeBoundary.halfHeight);
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
    mainWindow.create(sf::VideoMode(1366, 768), "Elastic Collisions", sf::Style::Titlebar); // (1366, 768) (1920, 1080)
    mainWindow.setFramerateLimit(60);
    sf::Event mainEvent;
    sf::Clock Timer;
    std::vector<particle> particles;
    std::vector<particle *> bucket;
    node quadTree(boundary(1366.0f / 2, 768.0f / 2, 1366.0f / 2, 768.0f / 2), 8, 5);
    bool quadTree_visibility = false;
    /* Switch
    {
        particle particle1(200.0f, 200.0f, 60.0f, sf::Vector2f(5.0f, 5.0f),99999.0f);
        particle1.setColor(sf::Color(209, 96, 61, 255));
        particles.push_back(particle1);
    }
    // */
    while (mainWindow.isOpen())
    {
        Timer.restart();
        while (mainWindow.pollEvent(mainEvent))
        {
            switch (mainEvent.type)
            {
            // case sf::Event::Resized:
            //     std::clog << "Main Window Resized now " << mainEvent.size.width << " by " << mainEvent.size.height << " pixels\n";
            //     break;
            case sf::Event::TextEntered:
                if (mainEvent.text.unicode > 31 && mainEvent.text.unicode < 128)
                {
                    std::clog << static_cast<char>(mainEvent.text.unicode) << "\n";
                }
                else if (mainEvent.text.unicode == 27)
                {
                    mainWindow.close();
                }
                break;
            case sf::Event::LostFocus:
                std::clog << "Main Window Unfocussed\n";
                break;
            case sf::Event::GainedFocus:
                std::clog << "Main Window Refocussed\n";
                break;
            }
        }
        // Take Input           note: All pointers to the particles vector array elements break here
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mainWindow.hasFocus())
        {
            float randNum = randomNum();
            particles.push_back(particle(sf::Mouse::getPosition(mainWindow).x, sf::Mouse::getPosition(mainWindow).y, randomNum() * 10.0f + 5.0f, sf::Vector2f(std::cos(randomNum() * 𝜏) * randNum * 2, std::sin(randNum * 𝜏) * randomNum() * 2)));
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
            {
                particles.back().setColor(sf::Color(254, 185, 95, 255));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            {
                particles.back().setColor(sf::Color(156, 56, 72, 255));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
            {
                particles.back().setColor(sf::Color(60, 145, 230, 255));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
            {
                particles.back().setColor(sf::Color(122, 199, 79, 255));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
            {
                particles.back().setColor(sf::Color(188, 150, 230, 255));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V))
            {
                particles.back().setColor(sf::Color(47, 0, 79, 255));
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
            {
                particles.back().setColor(sf::Color(209, 96, 61, 255));
            }
            else
            {
                particles.back().setColor(sf::Color(255, 255, 255, 255));
            }
        }
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && mainWindow.hasFocus())
        {
            for (size_t i = 0; i < particles.size(); ++i)
            {
                if (particles[i].distMouse() < particles[i].getRadius())
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
                    {
                        particles[i].setColor(sf::Color(254, 185, 95, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                    {
                        particles[i].setColor(sf::Color(156, 56, 72, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
                    {
                        particles[i].setColor(sf::Color(60, 145, 230, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
                    {
                        particles[i].setColor(sf::Color(122, 199, 79, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
                    {
                        particles[i].setColor(sf::Color(188, 150, 230, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V))
                    {
                        particles[i].setColor(sf::Color(47, 0, 79, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
                    {
                        particles[i].setColor(sf::Color(209, 96, 61, 255));
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    {
                        particles[i].setColor(sf::Color(255, 255, 255, 255));
                    }
                    else
                    {
                        particles.erase(begin(particles) + i);
                    }
                }
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace) && particles.size() > 0 && mainWindow.hasFocus())
        {
            particles.erase(begin(particles));
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Slash) && mainWindow.hasFocus())
        {
            std::cout << "Total particles are " << particles.size() << "\n";
            double total_KE = 0;
            for (particle i : particles)
            {
                total_KE += i.getKE();
            }
            std::cout << "Total Kinetic Energy of the system is " << total_KE << "\n";
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && mainWindow.hasFocus())
        {
            if (quadTree_visibility)
                quadTree_visibility = false;
            else
                quadTree_visibility = true;
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            {
                /* wait */
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) && mainWindow.hasFocus())
        {
            particles.clear();
        }

        // Make New Pointers Here for each game loop cycle
        // Calculate and Display
        mainWindow.clear(sf::Color(25, 30, 40, 225));
        /* switch      Old Method                   note: Good for under 500 particles
        for (size_t i = 0; i < particles.size(); ++i)
        {
            for (size_t j = i + 1; j < particles.size(); ++j)
            {
                particles[i].bodyCollision(particles[j]);
            }
        }
        //*/
        // /* switch      QuadTree Method          note: Good for about 2000 particles
        for (particle &i : particles)
        {
            quadTree.nestPoint(i);
        }
        if (quadTree_visibility)
        {
            quadTree.drawTree();
        }
        for (particle &i : particles)
        {
            quadTree.query(boundary(i.getX(), i.getY(), i.getRadius() + 20.0f, i.getRadius() + 20.0f), bucket); // Range boundary must be slightly more than radius of this ball + radius of largest possible ball
            for (particle *j : bucket)
            {
                if (&i != j)
                {
                    i.bodyCollision(*j);
                }
            }
            bucket.clear();
        }
        // */
        quadTree.clearTree();
        for (particle &i : particles)
        {
            i.edgeCollision();
            i.update();
            i.display();
        }
        mainWindow.display();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && mainWindow.hasFocus())
        {
            std::clog << "ms:" << 1000000 / 60 - Timer.getElapsedTime().asMicroseconds() << std::endl;
        }
        while (Timer.getElapsedTime().asMicroseconds() < 1000000 / 60) // One Second = 10^6 Micro Seconds
        {
            /* Wait */
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
    Forward Slash Button    : Info
    Space Bar               : tells how much early or late for this frame in micro seconds

    Last Edited on 31th Oct 2024
*/