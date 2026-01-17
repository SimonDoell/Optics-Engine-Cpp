#pragma once
#include "librarys.h"
#include "functions.h"


bool renderRayDebug = false;  //  Laggy, as it renders more than ~500000 circles per frame with 1000 rays
sf::Color debugColor = sf::Color(255, 255, 255, 10);


// Mouse Interaction
struct MouseSelection {
    public:
        virtual sf::Vector2f getPosForMouseSelection() {return {0, 0};}
        virtual void setNewMousePos(const sf::Vector2f& newMousePos) {}
};


struct DragPoint : public MouseSelection {
    public:
        sf::Vector2f& dragPoint;

        DragPoint(sf::Vector2f& _dragPoint) : dragPoint(_dragPoint) {}

        sf::Vector2f getPosForMouseSelection() override {return dragPoint;}
        void setNewMousePos(const sf::Vector2f& newMousePos) {dragPoint = newMousePos;}
};

struct DirPoint : public MouseSelection {
    public:
        sf::Vector2f& originDir;
        sf::Vector2f posOffset;
        float lenFactor;

        DirPoint(sf::Vector2f& _originDir, sf::Vector2f _posOffset, float _lenFactor = 150.0f) : originDir(_originDir), posOffset(_posOffset), lenFactor(_lenFactor) {}

        sf::Vector2f getPosForMouseSelection() override {return posOffset + originDir*lenFactor;}
        void setNewMousePos(const sf::Vector2f& newMousePos) {originDir = -normalize(posOffset - newMousePos);}
};

struct RadiusPoint : public MouseSelection {
    public:
        float& originRadius;
        sf::Vector2f posOffset;
        float radiusFactor = 1.0f;
        float pointAngle;

        RadiusPoint(float& _originRadius, sf::Vector2f _posOffset, float _radiusFactor = 1.0f, float _pointAngle = 0.0f) : originRadius(_originRadius), posOffset(_posOffset), radiusFactor(_radiusFactor), pointAngle(_pointAngle) {}

        sf::Vector2f getPosForMouseSelection() override {
            return posOffset + sf::Vector2f(cos(pointAngle)*originRadius/radiusFactor, sin(pointAngle)*originRadius/radiusFactor);
        }

        void setNewMousePos(const sf::Vector2f& newMousePos) {originRadius = len(posOffset - newMousePos)*radiusFactor;}
};




// Light-Ray
struct Ray {
    public:
        sf::Vector2f origin;
        sf::Vector2f dir;
        float waveLen = 630;

        Ray(sf::Vector2f _origin, sf::Vector2f _dir) : origin(_origin), dir(normalize(_dir)) {}
};

struct Emitter {
    public:
        sf::Vector2f pos;
        sf::Vector2f dir;
        int amountRays;
        float width;
        float angleSpread;
        bool isStatic = false;

        Emitter(sf::Vector2f _pos, sf::Vector2f _dir, float _width, float _angleSpread, int _amountRays = 10, bool _isStatic = false) : pos(_pos), dir(_dir), width(_width), angleSpread(_angleSpread), amountRays(_amountRays), isStatic(_isStatic) {}

        std::vector<Ray> getRays() {
            if (amountRays > 1) {
                std::vector<Ray> rays;
                sf::Vector2f scatterNormal = sf::Vector2f(dir.y, -dir.x);
                sf::Vector2f startPos = pos - scatterNormal*width/2.0f;

                for (int i = 0; i < amountRays; ++i) {
                    rays.emplace_back(Ray(
                        startPos + scatterNormal*((float)i/((float)amountRays - 1))*width,
                        dir
                    ));
                }
                return rays;
            } else {
                return {Ray(pos, dir)};
            }
        }

        std::vector<MouseSelection*> getPossibleMouseSelections() {
            std::vector<MouseSelection*> selections;

            selections.emplace_back(new DragPoint(pos));
            selections.emplace_back(new DirPoint(dir, pos));

            return selections;
        }
};




// Hitbox
struct Hitbox {
    public:
        virtual float getSignedDistToHitbox(const sf::Vector2f& p) {return 0.0f;}
        virtual sf::Vector2f getNormalAtPos(const sf::Vector2f& p) {return {0.0f, 0.0f};}
        virtual void render(sf::RenderWindow& window) {}
        virtual std::vector<MouseSelection*> getPossibleMouseSelections() {return {};}
        ////virtual float getSquaredSignedDistToHitbox(const sf::Vector2f& p) {return 0.0f;}
};

struct CircleHitbox : public Hitbox {
    public:
        sf::Vector2f pos;
        float r;

        CircleHitbox(sf::Vector2f _pos, float _r) : pos(_pos), r(_r) {}

        float getSignedDistToHitbox(const sf::Vector2f& p) override {return len(pos - p) - r;}
        sf::Vector2f getNormalAtPos(const sf::Vector2f& p) override {return normalize(pos - p);}
        void render(sf::RenderWindow& window) override {
            sf::CircleShape obj(r);
            obj.setOrigin(r, r);
            obj.setPosition(pos);
            obj.setFillColor(sf::Color::Transparent);
            obj.setOutlineColor(sf::Color::Blue);
            obj.setOutlineThickness(2);
            obj.setPointCount(100);

            window.draw(obj);
        }

        std::vector<MouseSelection*> getPossibleMouseSelections() {
            std::vector<MouseSelection*> selections;

            selections.emplace_back(new DragPoint(pos));
            selections.emplace_back(new RadiusPoint(r, pos, 1.0f, 2.0f*PI/3.0f*0.0f));
            selections.emplace_back(new RadiusPoint(r, pos, 1.0f, 2.0f*PI/3.0f*1.0f));
            selections.emplace_back(new RadiusPoint(r, pos, 1.0f, 2.0f*PI/3.0f*2.0f));

            return selections;
        }
};

struct LineHitbox : public Hitbox {
    public:
        sf::Vector2f pos;
        sf::Vector2f dir;
        float lineLen;

        LineHitbox(sf::Vector2f _pos, sf::Vector2f _dir, float _lineLen) : pos(_pos), dir(normalize(_dir)), lineLen(_lineLen) {}

        float getSignedDistToHitbox(const sf::Vector2f& p) override {
            sf::Vector2f alongLineDir = normalize(sf::Vector2f(dir.y, -dir.x));
            sf::Vector2f cornerA = pos + alongLineDir*lineLen/2.0f;
            sf::Vector2f cornerB = pos - alongLineDir*lineLen/2.0f;

            sf::Vector2f relativ = p - pos;
            float otherSignedDist = INFINITY;
            sf::Vector2f cNormal = dir; if (dot(relativ, cNormal) < 0) {cNormal = -cNormal;}
            if (dot(alongLineDir, relativ) < lineLen/2.0f && dot(alongLineDir, relativ) > -lineLen/2.0f) {
                otherSignedDist = dot(cNormal, relativ);
            }

            return std::min(std::min(len(cornerA - p), len(cornerB - p)), otherSignedDist);
        }

        sf::Vector2f getNormalAtPos(const sf::Vector2f& p) override {
            sf::Vector2f cNormal = dir;
            if (dot(p, cNormal) < 0) {cNormal = -cNormal;}

            return cNormal;
        }

        void render(sf::RenderWindow& window) override {
            sf::Vector2f alongLineDir = normalize(sf::Vector2f(dir.y, -dir.x));
            Line line(pos + alongLineDir*lineLen/2.0f, pos - alongLineDir*lineLen/2.0f, sf::Color::Blue, 2.0f);
            line.render(window);
        }

        std::vector<MouseSelection*> getPossibleMouseSelections() {
            std::vector<MouseSelection*> selections;

            selections.emplace_back(new DragPoint(pos));
            selections.emplace_back(new DirPoint(dir, pos, 150.0f));
            selections.emplace_back(new RadiusPoint(lineLen, pos, 2.0f, -atan2(dir.x, dir.y)));
            selections.emplace_back(new RadiusPoint(lineLen, pos, 2.0f, 1.5f*PI-atan2(dir.y, -dir.x)));

            return selections;
        }
};
//----------



// Interactions
struct Interaction {
    public:
        virtual void interact(Ray& r, const sf::Vector2f& normal, const sf::Vector2f& hitPoint) {}
};

struct Reflection : public Interaction {
    public:
        void interact(Ray& r, const sf::Vector2f& normal, const sf::Vector2f& hitPoint) {
            sf::Vector2f& rayDir = r.dir;

            rayDir = normalize(rayDir - 2.0f * dot(rayDir, normal) * normal);
        }
};
//----------


struct Object {
    public:
        Hitbox* hitbox;
        Interaction* interaction;
        bool isStatic = false;

        Object(Hitbox* _hitbox, Interaction* _interaction, bool _isStatic = false) : hitbox(_hitbox), interaction(_interaction), isStatic(_isStatic) {}
};




struct Scene {
    private:
        float outOfBoundsPadding = 100.0f;
        bool rayOutOfBounds(const sf::Vector2f& p) {
            if (p.x < -outOfBoundsPadding || p.y < -outOfBoundsPadding || p.x > WIDTH+outOfBoundsPadding || p.y > HEIGHT+outOfBoundsPadding) return true;
            else return false;
        }

        float minRayStep = 1.0f;
        void renderRay(Ray& r, sf::RenderWindow& window) {
            sf::Vector2f rPos = r.origin;
            sf::Vector2f lastInteractPos = r.origin;

            if (objects.size() > 0) {
                int iteration = 0;
                bool internalCollisionDetected = false;

                while (!rayOutOfBounds(rPos) && iteration < 1000 && !internalCollisionDetected) {
                    float minSignedDist = INFINITY;

                    for (Object& o : objects) {
                        float singedDist = o.hitbox->getSignedDistToHitbox(rPos);
                        if (singedDist < minSignedDist) {minSignedDist = singedDist;}

                        if (singedDist < minRayStep) {
                            Line line(rPos, lastInteractPos, sf::Color::Red, 2.0f);
                            line.render(window);

                            o.interaction->interact(r, o.hitbox->getNormalAtPos(rPos), rPos);
                            lastInteractPos = rPos;

                            if (singedDist < -minRayStep) {
                                internalCollisionDetected = true;
                            }
                        }
                    }

                    float distStep = std::max(minSignedDist, minRayStep);
                    if (renderRayDebug) {
                        sf::CircleShape obj(distStep - 2);
                        obj.setPosition(rPos);
                        obj.setFillColor(sf::Color::Transparent);
                        obj.setPointCount(100);
                        obj.setOrigin(distStep - 2, distStep - 2);
                        obj.setOutlineColor(debugColor);
                        obj.setOutlineThickness(2);

                        window.draw(obj);
                    }
                    rPos += r.dir * distStep;
                    iteration++;
                }

                Line line(rPos, lastInteractPos, sf::Color::Red, 2.0f);
                line.render(window);
            } else {
                Line line(rPos + (r.dir*1000.0f), rPos, sf::Color::Red, 2.0f);
                line.render(window);
            }
        }

        // Rendering
        void renderRays(sf::RenderWindow& window) {
            for (Emitter& e : emitters) {
                for (Ray& r : e.getRays()) {
                    renderRay(r, window);
                }
            }
        }

        void renderEmitters(sf::RenderWindow& window) {
            for (Emitter& e : emitters) {
                sf::Vector2f alongMirror = normalize(sf::Vector2f(e.dir.y, -e.dir.x));
                Line line(e.pos + alongMirror/2.0f*e.width, e.pos - alongMirror/2.0f*e.width, sf::Color::White);
                line.render(window);
            }
        }

        void renderObjects(sf::RenderWindow& window) {
            for (Object& o : objects) {o.hitbox->render(window);}
        }

        float mouseSelectR = 15.0f;

        // Mouse
        bool lDown = false;
        sf::Vector2f mousePos;
        float minMouseDistForSelection = 50.0f;
        MouseSelection* currentMouseSelection = nullptr;
        std::vector<MouseSelection*> possibleSelections;

        void findNewMouseSelection() {
            if (possibleSelections.size() < 1) return;

            float smallestDistance = INFINITY;
            MouseSelection* currentTempSelection = nullptr;

            for (MouseSelection* m : possibleSelections) {
                float distance = len(m->getPosForMouseSelection() - mousePos);
                if (distance < smallestDistance && distance < minMouseDistForSelection) {
                    smallestDistance = distance;
                    currentTempSelection = m;
                }
            }

            if (smallestDistance == INFINITY) return;

            currentMouseSelection = currentTempSelection;
        }

        // Keyboard
        bool tabDown = false;

    public:
        std::vector<Emitter> emitters;
        std::vector<Object> objects;

        Scene() {}

        void render(sf::RenderWindow& window) {
            renderRays(window);
            renderEmitters(window);
            renderObjects(window);
        }

        void mouseUpdateAndRender(sf::RenderWindow& window) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {if (!tabDown) {renderRayDebug = !renderRayDebug;} tabDown = true;} else {tabDown = false;}
            possibleSelections.clear();
            for (Emitter& e : emitters) { if (e.isStatic) continue; for (MouseSelection* m : e.getPossibleMouseSelections()) { possibleSelections.emplace_back(m); }}
            for (Object& o : objects) { if (o.isStatic) continue; for (MouseSelection* m : o.hitbox->getPossibleMouseSelections()) { possibleSelections.emplace_back(m); }}

            mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (!lDown) {
                    findNewMouseSelection();
                }
                lDown = true;
            } else {
                lDown = false;
                currentMouseSelection = nullptr;
            }

            if (!lDown) {
                sf::CircleShape obj(mouseSelectR);
                obj.setOrigin(mouseSelectR, mouseSelectR);
                obj.setPointCount(4);
                obj.setFillColor(sf::Color::Transparent);
                obj.setOutlineColor(sf::Color::Green);
                obj.setOutlineThickness(2);

                for (MouseSelection* m : possibleSelections) {
                    obj.setPosition(m->getPosForMouseSelection());
                    window.draw(obj);
                }
            }

            // Updating
            if (currentMouseSelection != nullptr) {
                currentMouseSelection->setNewMousePos(mousePos);
            }
        }
};