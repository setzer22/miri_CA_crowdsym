#pragma once

struct Entity {
    float x;
    float y;
    float size;
};

Entity make_random_entity() {
    Entity e;
    e.x = (float)(rand() % 640);
    e.y = (float)(rand() % 480);
    e.size = (float)(rand() % 50);
    return e;
}
