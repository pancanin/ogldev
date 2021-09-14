/*

        Copyright 2021 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Tutorial 20 - Diffuse Lighting
*/

#include <stdio.h>
#include <string.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "ogldev_world_transform.h"
#include "ogldev_basic_mesh.h"
#include "camera.h"
#include "lighting_technique.h"
#include "ogldev_engine_common.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440



class Tutorial20
{
public:
    Tutorial20();
    ~Tutorial20();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;
    BasicMesh* pMesh = NULL;
    PersProjInfo persProjInfo;
    LightingTechnique* pLightingTech = NULL;
    DirectionalLight dirLight;
};


Tutorial20::Tutorial20()
{
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 100.0f;

    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

    dirLight.AmbientIntensity = 0.1f;
    dirLight.DiffuseIntensity = 1.0f;
    dirLight.Direction = Vector3f(1.0f, 0.0, 0.0);
}


Tutorial20::~Tutorial20()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (pMesh) {
        delete pMesh;
    }

    if (pLightingTech) {
        delete pLightingTech;
    }
}


bool Tutorial20::Init()
{
    Vector3f CameraPos(0.0f, 0.0f, -1.0f);
    Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    pMesh = new BasicMesh();

    if (!pMesh->LoadMesh("../Content/box.obj")) {
        return false;
    }

    pLightingTech = new LightingTechnique();

    if (!pLightingTech->Init())
    {
        return false;
    }

    pLightingTech->Enable();

    pLightingTech->SetTextureUnit(COLOR_TEXTURE_UNIT);

    return true;
}


void Tutorial20::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

#ifdef _WIN64
    float YRotationAngle = 0.1f;
#else
    float YRotationAngle = 1.0f;
#endif

    WorldTrans& worldTransform = pMesh->GetWorldTransform();

    worldTransform.SetScale(1.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 2.0f);
    //    worldTransform.SetRotation(0.0f, 45.0f, 0.0f);
    worldTransform.Rotate(0.0f, YRotationAngle, 0.0f);

    Matrix4f World = worldTransform.GetMatrix();

    Matrix4f WorldTranspose = World.Transpose();
    printf("World\n");
    World.Print();
    printf("----\n");
    printf("Transpose\n");
    WorldTranspose.m[0][3] = 0.0f;
    WorldTranspose.m[1][3] = 0.0f;
    WorldTranspose.m[2][3] = 0.0f;
    WorldTranspose.m[3][3] = 0.0f;
    WorldTranspose.m[3][0] = 0.0f;
    WorldTranspose.m[3][1] = 0.0f;
    WorldTranspose.m[3][2] = 0.0f;
    WorldTranspose.Print();
    printf("----\n");

    Vector3f foo(1.0f, 0.0f, 0.0f);
    Vector4f LightDirLocal = WorldTranspose * Vector4f(foo, 0.0f);
    LightDirLocal.Print();
    //    exit(0);
    dirLight.Direction = Vector3f(LightDirLocal.x, LightDirLocal.y, LightDirLocal.z);

    Matrix4f View = pGameCamera->GetMatrix();

    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);

    Matrix4f WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);
    //    pLightingTech->SetWorldMatrix(World);
    pLightingTech->SetDirectionalLight(dirLight);
    pLightingTech->SetMaterial(pMesh->GetMaterial());

    pMesh->Render();

    glutPostRedisplay();

    glutSwapBuffers();
}


void Tutorial20::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case 'q':
    case 27:    // escape key code
        exit(0);
    }

    pGameCamera->OnKeyboard(key);
}


void Tutorial20::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void Tutorial20::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


Tutorial20* pTutorial20 = NULL;


void RenderSceneCB()
{
    pTutorial20->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pTutorial20->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pTutorial20->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pTutorial20->PassiveMouseCB(x, y);
}


void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutKeyboardFunc(KeyboardCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
}

int main(int argc, char** argv)
{
#ifdef _WIN64
    srand(GetCurrentProcessId());
#else
    srandom(getpid());
#endif

    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Tutorial 18");
    printf("window id: %d\n", win);

    char game_mode_string[64];
    // Game mode string example: 2020x1080@32
    // Enable the following three lines for full screen
    // snprintf(game_mode_string, sizeof(game_mode_string), "%dx%d@32", WINDOW_WIDTH, WINDOW_HEIGHT);
    // glutGameModeString(game_mode_string);
    // glutEnterGameMode();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    InitializeGlutCallbacks();

    pTutorial20 = new Tutorial20();

    if (!pTutorial20->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
