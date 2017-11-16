#pragma once

#include "ObjLoader.h"

namespace renderer20 {

    void render_model(Model* model) {
        GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat mat_shininess[] = { 50.0 };
        GLfloat light_position[] = { 1.0, -1.0, -1.0, 0.0 };
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_MODELVIEW);
        glTranslated(1.0, 1.0, 0.0);

        const Model& m = *model;
        for (int i = 0; i < m.faces.size; ++i) {

            const Face& f = m.faces[i];
            const Vector3& v1 = m.vertices[f.v1];
            const Vector3& v2 = m.vertices[f.v2];
            const Vector3& v3 = m.vertices[f.v3];
            const Vector3& n1 = m.normals[f.n1];
            const Vector3& n2 = m.normals[f.n2];
            const Vector3& n3 = m.normals[f.n3];

            glBegin(GL_TRIANGLES);

            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f(v1.x, v1.y, v1.z);

            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f(v2.x, v2.y, v2.z);

            glNormal3f(n3.x, n3.y, n3.z);
            glVertex3f(v3.x, v3.y, v3.z);
            
            glEnd();
        }
        glLoadIdentity();
    }

}
