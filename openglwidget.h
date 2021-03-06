#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QtOpenGL>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>

#include <memory>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

    GLuint vboNormals = 0;
    GLuint vboVertices = 0;
    GLuint vboColors = 0;
    GLuint vboIndices = 0;

    GLuint vao = 0;
    GLuint vao2 = 0;

    std::unique_ptr<QVector3D []> normals;
    std::unique_ptr<QVector4D []> vertices;
    std::unique_ptr<QVector4D []> colors = nullptr;
    std::unique_ptr<unsigned int[]> indices = nullptr;

    GLuint shaderProgram;

    float playerPosYOffset; // Player displacement along Y axis
    float playerPosY; // Current player Y position
    float playerPosXOffset; // Player displacement along Y axis
    float playerPosX; // Current player Y position

    float targetPosYOffset; // Target displacement along Y axis
    float targetPosY; // Current target Y position
    float targetPosXOffset; // Target displacement along Y axis
    float targetPosX; // Current target Y position

    float targetRandX;
    float targetRandY;

    bool dead; // Whether the projectile is being animated

    int numHits; // Number of hits
    int numLevel; // Number of hits
    float numExp;

    QTimer timer;
    QTime time;

public:
    explicit OpenGLWidget (QWidget *parent = 0);

    void createNormals();

    void createVBOs();
    void createShaders();

    void destroyVBOs();
    void destroyShaders();

    std::unique_ptr <QVector2D []> texCoords;
    void createTexCoords();
    GLuint vboTexCoords = 0;
    GLuint textureID = 0;
    GLuint texturas[2];
    void loadTexture();

protected:
    void initializeGL();
    void resizeGL (int width, int height);
    void paintGL();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

signals:
    void updateHitsLabel(QString);
    void updateHP(int);
    void updateLevel(int);

public slots:
    void animate();
};
#endif
