#pragma once

bool init_GL() {
    bool success = true;
    GLenum error = GL_NO_ERROR;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);

    error = glGetError();
    if (error != GL_NO_ERROR) success = false;

    return success;
}

void ortho(float left, float right, float top, float bottom, float znear,
           float zfar) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left, right, bottom, top, znear, zfar);
    glMatrixMode(GL_MODELVIEW);
}

void quad(float x1, float y1, float u1, float v1, float x2, float y2, float u2,
          float v2, float x3, float y3, float u3, float v3, float x4, float y4,
          float u4, float v4) {
    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex3f(x1, y1, 0);
    glTexCoord2f(u2, v2);
    glVertex3f(x2, y2, 0);
    glTexCoord2f(u3, v3);
    glVertex3f(x3, y3, 0);
    glTexCoord2f(u4, v4);
    glVertex3f(x4, y4, 0);
    glEnd();
}

void rect(float x, float y, float w, float h) {
    quad(x + 0, y + 0, 0.0f, 0.0f, x + w, y + 0, 1.0f, 0.0f, x + w, y + h, 1.0f,
         1.0f, x + 0, y + h, 0.0f, 1.0f);
}

void center_rect(float x, float y, float w, float h) {
    float w2 = w / 2;
    float h2 = h / 2;
    quad(x - w2, y - h2, 0.0f, 0.0f, x + w2, y - h2, 1.0f, 0.0f, x + w2, y + h2,
         1.0f, 1.0f, x - h2, y + h2, 0.0f, 1.0f);
}
