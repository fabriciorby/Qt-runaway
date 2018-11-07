#include "openglwidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    playerPosYOffset = 0;
    playerPosY = 0;
    playerPosXOffset = 0;
    playerPosX = -0.8f;

    targetPosYOffset = 2.0f;
    targetPosY = 0;

    targetPosXOffset = 2.0f;
    targetPosX = 0;

    targetRandX = 1.0f;
    targetRandY = 1.0f;

    dead = false;

    numHits = 0;
    numLevel = 1;
    numExp = 0;
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    qDebug("OpenGL version: %s", glGetString(GL_VERSION));
    qDebug("GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    createShaders();
    createVBOs();

    connect(&timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer.start(0);

    time.start();
}

void OpenGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint locScaling = glGetUniformLocation(shaderProgram, "scaling");
    GLuint locTranslation = glGetUniformLocation(shaderProgram, "translation");

    glUseProgram(shaderProgram);

    glBindVertexArray(vao);           

    // Player
    glUniform4f(locTranslation, playerPosX, playerPosY, 0, 0);
    glUniform1f(locScaling, 0.2);
    glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0);

    // Target
    glUniform4f(locTranslation, targetPosX, targetPosY, 0, 0);
    glUniform1f(locScaling, 0.2);
    glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0);

}

void OpenGLWidget::createShaders()
{
    // makeCurrent();
    destroyShaders();

    QFile vs(":/shaders/vshader1.glsl");
    QFile fs(":/shaders/fshader1.glsl");

    vs.open(QFile::ReadOnly | QFile::Text);
    fs.open(QFile::ReadOnly | QFile::Text);

    QTextStream streamVs(&vs), streamFs(&fs);

    QString qtStringVs = streamVs.readAll();
    QString qtStringFs = streamFs.readAll();

    std::string stdStringVs = qtStringVs.toStdString();
    std::string stdStringFs = qtStringFs.toStdString();

    // Create an empty vertex shader handle
    GLuint vertexShader = 0;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Send the vertex shader source code to GL
    const GLchar *source = stdStringVs.c_str();

    glShaderSource(vertexShader, 1, &source, 0);

    // Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        qDebug("%s", &infoLog[0]);

        glDeleteShader(vertexShader);
        return;
    }

    // Create an empty fragment shader handle
    GLuint fragmentShader = 0;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Send the fragment shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    source = stdStringFs.c_str();
    glShaderSource(fragmentShader, 1, &source, 0);

    // Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        qDebug("%s", &infoLog[0]);

        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        return;
    }

    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    shaderProgram = glCreateProgram();

    // Attach our shaders to our program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Link our program
    glLinkProgram(shaderProgram);

    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
        qDebug("%s", &infoLog[0]);

        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    vs.close();
    fs.close();
}

void OpenGLWidget::destroyShaders()
{
    makeCurrent();
    glDeleteProgram(shaderProgram);
}

void OpenGLWidget::createVBOs()
{
    makeCurrent();
    destroyVBOs();

    vertices = std::make_unique<QVector4D[]>(4);
    colors = std::make_unique<QVector4D[]>(4);
    indices = std::make_unique<unsigned int[]>(2 * 3);

    vertices[0] = QVector4D(-0.5, -0.5, 0, 1);
    vertices[1] = QVector4D( 0.5, -0.5, 0, 1);
    vertices[2] = QVector4D( 0.5,  0.5, 0, 1);
    vertices[3] = QVector4D(-0.5,  0.5, 0, 1);

    colors[0] = QVector4D(0, 1, 0, 1); // green
    colors[1] = QVector4D(0, 1, 0, 1); // green
    colors[2] = QVector4D(0, 1, 0, 1); // green
    colors[3] = QVector4D(0, 1, 0, 1); // green

//    colors[0] = QVector4D(1, 0, 0, 1); // red
//    colors[1] = QVector4D(0, 1, 0, 1); // green
//    colors[2] = QVector4D(0, 0, 1, 1); // blue
//    colors[3] = QVector4D(1, 1, 0, 1); // yellow

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(QVector4D), vertices.get(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(QVector4D), colors.get(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(unsigned int), indices.get(), GL_DYNAMIC_DRAW);
}

void OpenGLWidget::destroyVBOs()
{
    glDeleteBuffers(1, &vboVertices);
    glDeleteBuffers(1, &vboColors);
    glDeleteBuffers(1, &vboIndices);
    glDeleteVertexArrays(1, &vao);

    vboVertices = 0;
    vboIndices = 0;
    vboColors = 0;
    vao = 0;
}

void OpenGLWidget::animate()
{

    if (!dead) {

        float elapsedTime = time.restart() / 1000.0f;

        // Change player position
        playerPosY += playerPosYOffset * elapsedTime;
        playerPosX += playerPosXOffset * elapsedTime;

        // Check player bounds
        if (playerPosY < -0.8f)
            playerPosY = -0.8f;
        if (playerPosY > 0.8f)
            playerPosY = 0.8f;
        if (playerPosX < -0.8f)
            playerPosX = -0.8f;
        if (playerPosX > 0.8f)
            playerPosX = 0.8f;

        // Update target
        targetPosY += targetPosYOffset * elapsedTime * targetRandY;
        targetPosX += targetPosXOffset * elapsedTime * targetRandX;

        numExp += elapsedTime * 10.0f;
        emit updateLevel(numExp);

        if (numExp >= 100) {
            numLevel++;
            numExp = 0;
            emit updateHitsLabel(QString("Level %1").arg(numLevel));
        }

        if (std::fabs(playerPosY - targetPosY) < 0.125f && std::fabs(playerPosX - targetPosX) < 0.125f)
        {
            numHits++;

            if (numHits >= 25)
            {
                qDebug("Morreu!");

                emit updateHitsLabel(QString("Level %1 - Morreu!").arg(numLevel));

                dead = true;

                playerPosXOffset = 0;
                playerPosYOffset = 0;
                targetPosXOffset = 0;
                targetPosYOffset = 0;
            }

            emit updateHP(25 - numHits);

        }

        if (targetPosYOffset > 0)
        {
            if (targetPosY > 0.8f)
            {
                targetPosY = 0.8f;
                targetPosYOffset = -targetPosYOffset;
                targetRandY = (qrand() / (float)RAND_MAX) + 1.0f;
            }
        }
        else if (targetPosYOffset < 0)
        {
            if (targetPosY < -0.8f)
            {
                targetPosY = -0.8f;
                targetPosYOffset = -targetPosYOffset;
                targetRandY = (qrand() / (float)RAND_MAX) + 1.0f;
            }
        }

        if (targetPosXOffset > 0)
        {
            if (targetPosX > 0.8f)
            {
                targetPosX = 0.8f;
                targetPosXOffset = -targetPosXOffset;
                targetRandX = (qrand() / (float)RAND_MAX) + 1.0f;
            }
        }
        else if (targetPosXOffset < 0)
        {
            if (targetPosX < -0.8f)
            {
                targetPosX = -0.8f;
                targetPosXOffset = -targetPosXOffset;
                targetRandX = (qrand() / (float)RAND_MAX) + 1.0f;
            }
        }

    update();
    }
}

// Strong focus is required
void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_W)
        playerPosYOffset = 2.0f;

    if (event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_S)
        playerPosYOffset = -2.0f;

    if (event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_D)
        playerPosXOffset = 2.0f;

    if (event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_A)
        playerPosXOffset = -2.0f;

    if (event->key() == Qt::Key_Escape)
    {
        QApplication::quit();
    }
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_W)
        playerPosYOffset = 0;

    if (event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_S)
        playerPosYOffset = 0;

    if (event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_D)
        playerPosXOffset = 0;

    if (event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_A)
        playerPosXOffset = 0;
}
