#include "librarys.h"
#include "classes.h"



int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Optics Engine V3", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMES);
    sf::Event ev;
    sf::ContextSettings settings;
    sf::View view(sf::FloatRect(0, 0, WIDTH, HEIGHT));
    view.zoom(1.0f);
    window.setView(view);
    int frame = 0;


    Scene scene;
    scene.emitters.emplace_back(Emitter({WIDTH/2.0f, HEIGHT/2.0f}, {1, 0}, 10.0f, 0.0f, 1));

    scene.objects.emplace_back(Object(
        new CircleHitbox({WIDTH/2.0f + 400.0f, HEIGHT/2.0f}, 50.0f),
        new Reflection
    ));

    //scene.objects.emplace_back(Object(
    //    new LineHitbox({WIDTH/2.0f - 400.0f, HEIGHT/2.0f}, {1, 1}, 100.0f),
    //    new Reflection
    //));



    while (window.isOpen()) {
        while (window.pollEvent(ev)) {if (ev.type == sf::Event::Closed) {window.close(); break;}}
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();


        // Rendering
        auto logStart = startTimer();
        if (frame % 30 == 0) logStart = startTimer();
        window.clear(bgColor);
        scene.render(window);
        scene.mouseUpdateAndRender(window);
        window.display();
        if (frame % 30 == 0) stopTimer(logStart, "Single Frame Rendering");
        frame++;
    }
    return 0;
}