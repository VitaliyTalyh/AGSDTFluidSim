#include <QGuiApplication>

#include "OpenGLWidget.h"
#include <iostream>
#include <time.h>

#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>
#include <ngl/Random.h>
#include <ngl/VertexArrayObject.h>
#include <ngl/ShaderLib.h>
#include <ngl/Random.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;

OpenGLWidget::OpenGLWidget(const QGLFormat _format, QWidget *_parent) : QGLWidget(_format,_parent){
    // set this widget to have the initial keyboard focus
    setFocus();
    setFocusPolicy( Qt::StrongFocus );
    //init some members
    m_rotate=false;
    // mouse rotation values set to 0
    m_spinXFace=0;
    m_spinYFace=0;
    m_modelPos=ngl::Vec3(0.0);
    //init our point size
    m_pointSize = 0.5f;
    // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
    this->resize(_parent->size());
}
//----------------------------------------------------------------------------------------------------------------------
OpenGLWidget::~OpenGLWidget(){
    ngl::NGLInit *Init = ngl::NGLInit::instance();
    std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
    Init->NGLQuit();

    //clean up everything
    glDeleteVertexArrays(1,&m_billboardVAO);
    glDeleteTextures(1,&m_depthRender);
    glDeleteFramebuffers(1,&m_depthFrameBuffer);
    glDeleteRenderbuffers(1,&m_staticDepthBuffer);

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::initializeGL(){

    // we must call this first before any other GL commands to load and link the
    // gl commands from the lib, if this is not done program will crash
    ngl::NGLInit::instance();

    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);
    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);
    //enable point sprites
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // as re-size is not explicitly called we need to do this.
    glViewport(0,0,width(),height());

    //used for drawing text later
    m_text = new ngl::Text(QFont("calibri",14));
    m_text->setColour(255,0,0);
    m_text->setScreenSize(width(),height());

    // Initialise the model matrix
    m_modelMatrix = ngl::Mat4(1.0);

    // Initialize the camera
    // Now we will create a basic Camera from the graphics library
    // This is a static camera so it only needs to be set once
    // First create Values for the camera position
    ngl::Vec3 from(0,10,40);
    ngl::Vec3 to(0,0,0);
    ngl::Vec3 up(0,1,0);
    m_cam= new ngl::Camera(from,to,up);
    // set the shape using FOV 45 Aspect Ratio based on Width and Height
    // The final two are near and far clipping planes of 0.1 and 100
    m_cam->setShape(45,(float)width()/height(),1,1000);

    //create our local frame buffer for our depth pass
    glGenFramebuffers(1,&m_depthFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthFrameBuffer);

    // The depth buffer so that we have a depth test when we render to texture
    glGenRenderbuffers(1, &m_staticDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_staticDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_staticDepthBuffer);

    //Create our depth texture to render to on the GPU
    glGenTextures(1, &m_depthRender);
    glBindTexture(GL_TEXTURE_2D, m_depthRender);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width(), height(), 0,GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //note poor filtering is needed to be accurate with pixels and have no smoothing
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set our render target to colour attachment 1
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_depthRender, 0);
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT1};
    glDrawBuffers(1, DrawBuffers);

    //check to see if the frame buffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr<<"Local framebuffer could not be created"<<std::endl;
        exit(-1);
    }

    //create our local frame buffer for our bilateral filter pass
    glGenFramebuffers(1,&m_bilateralFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_bilateralFrameBuffer);

    //create our bilateral texture on to render to on the GPU
    glGenTextures(1, &m_bilateralRender);
    glBindTexture(GL_TEXTURE_2D, m_bilateralRender);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width(), height(), 0,GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set our render target to colour attachment 1
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_bilateralRender, 0);
    // Set the list of draw buffers.
    glDrawBuffers(1, DrawBuffers);

    //check to see if the frame buffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr<<"Local framebuffer could not be created"<<std::endl;
        exit(-1);
    }


    //create our billboard geomtry
    float vertex[]={
        //bottom left
        -1.0f,-1.0f,
        //top left
        -1.0f,1.0f,
        //bottom right
        1.0f,-1.0f,
        //top left
        -1.0f,1.0f,
        //top right
        1.0f,1.0f,
        //bottom right
        1.0f,-1.0f
    };
    float texCoords[]={
        //bottom left
        0.0,0.0f,
        //top left
        0.0f,1.0f,
        //bottom right
        1.0f,0.0f,
        //top left
        0.0f,1.0f,
        //top right
        1.0f,1.0f,
        //bottom right
        1.0f,0.0f
    };

    glGenVertexArrays(1,&m_billboardVAO);
    glBindVertexArray(m_billboardVAO);
    GLuint billboardVBO[2];
    glGenBuffers(2, billboardVBO);
    glBindBuffer(GL_ARRAY_BUFFER,billboardVBO[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertex),vertex,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(float)*2.0,(GLvoid*)(0*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,billboardVBO[1]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(texCoords),texCoords,GL_STATIC_DRAW);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*2.0,(GLvoid*)(0*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);


    //set up our particle shader to render depth information to a texture
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    //first lets add out deformation shader to our program
    //create the program
    shader->createShaderProgram("ParticleDepth");
    //add our shaders
    shader->attachShader("particleDepthVert",ngl::VERTEX);
    shader->attachShader("particleDepthFrag",ngl::FRAGMENT);
    //load the source
    shader->loadShaderSource("particleDepthVert","shaders/particleDepthVert.glsl");
    shader->loadShaderSource("particleDepthFrag","shaders/particleDepthFrag.glsl");
    //compile them
    shader->compileShader("particleDepthVert");
    shader->compileShader("particleDepthFrag");
    //attach them to our program
    shader->attachShaderToProgram("ParticleDepth","particleDepthVert");
    shader->attachShaderToProgram("ParticleDepth","particleDepthFrag");
    //link our shader to opengl
    shader->linkProgramObject("ParticleDepth");

    //create our bilateral filter shader
    //creat program
    shader->createShaderProgram("BilateralFilter");
    //ass shaders
    shader->attachShader("bilateralFilterVert",ngl::VERTEX);
    shader->attachShader("bilateralFilterFrag",ngl::FRAGMENT);
    //load source
    shader->loadShaderSource("bilateralFilterVert","shaders/bilateralFilterVert.glsl");
    shader->loadShaderSource("bilateralFilterFrag","shaders/bilateralFilterFrag.glsl");
    //compile them
    shader->compileShader("bilateralFilterVert");
    shader->compileShader("bilateralFilterFrag");
    //attach them to our program
    shader->attachShaderToProgram("BilateralFilter","bilateralFilterVert");
    shader->attachShaderToProgram("BilateralFilter","bilateralFilterFrag");
    //link our shader to openGL
    shader->linkProgramObject("BilateralFilter");

    //create our water shader
    //create program
    shader->createShaderProgram("FluidShader");
    //add shaders
    shader->attachShader("fluidShaderVert",ngl::VERTEX);
    shader->attachShader("fluidShaderFrag",ngl::FRAGMENT);
    //load source
    shader->loadShaderSource("fluidShaderVert","shaders/fluidShaderVert.glsl");
    shader->loadShaderSource("fluidShaderFrag","shaders/fluidShaderFrag.glsl");
    //compile them
    shader->compileShader("fluidShaderVert");
    shader->compileShader("fluidShaderFrag");
    //attach them to our program
    shader->attachShaderToProgram("FluidShader","fluidShaderVert");
    shader->attachShaderToProgram("FluidShader","fluidShaderFrag");
    //link our shader to openGL
    shader->linkProgramObject("FluidShader");

    //set some uniforms
    (*shader)["ParticleDepth"]->use();

    (*shader)["BilateralFilter"]->use();
    shader->setUniform("depthTex",0);
    shader->setUniform("blurDir",1.0f,0.0f);
    shader->setUniform("blurDepthFalloff",0.1f);
    shader->setUniform("filterRadius",2.0f/width());
    shader->setUniform("texelSize",1.0f/width());

    (*shader)["FluidShader"]->use();
    //set our inverse projection matrix
    ngl::Mat4 P = m_cam->getProjectionMatrix();
    ngl::Mat4 PInv = P.inverse();
    shader->setUniform("PInv",PInv);
    shader->setUniform("depthTex",0);
    shader->setUniform("texelSizeX",1.0f/width());
    shader->setUniform("texelSizeY",1.0f/height());

    //to be used later with phone shading
    shader->setShaderParam3f("light.position",-1,-1,-1);
    shader->setShaderParam3f("light.intensity",0.8,0.8,0.8);
    shader->setShaderParam3f("Kd",0.5, 0.5, 0.5);
    shader->setShaderParam3f("Ka",0.5, 0.5, 0.5);
    shader->setShaderParam3f("Ks",1.0,1.0,1.0);
    shader->setShaderParam1f("shininess",100.0);


    //allocate some space for our SPHEngine
    m_SPHEngine = new SPHEngine(30000);
    m_SPHEngine->setVolume(10);
    //add some walls to our simulation
    m_SPHEngine->addWall(make_float3(0.0f,0.0f,0.0f),make_float3(0.0f,1.0f,0.0f));      //floor
    //m_SPHEngine->addWall(make_float3(-12.0f,0.0f,0.0f),make_float3(1.0f,0.0f,0.0f));    //left
    //m_SPHEngine->addWall(make_float3(12.0f,0.0f,0.0f),make_float3(-1.0f,0.0f,0.0f));    //right
    //m_SPHEngine->addWall(make_float3(0.0f,0.0f,12.0f),make_float3(0.0f,0.0f,-1.0f));    //front
    //m_SPHEngine->addWall(make_float3(0.0f,0.0f,-12.0f),make_float3(0.0f,0.0f,1.0f));    //back

    m_currentTime = m_currentTime.currentTime();
    startTimer(0);
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::resizeGL(const int _w, const int _h){
    // set the viewport for openGL
    glViewport(0,0,_w,_h);
    m_cam->setShape(45,(float)_w/_h, 1,1000);
    //resize our render targets and depth buffer
    glBindTexture(GL_TEXTURE_2D, m_depthRender);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, _w, _h, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, m_bilateralRender);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, _w, _h, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, m_staticDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    //update our texel sizes
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["FluidShader"]->use();
    ngl::Mat4 P = m_cam->getProjectionMatrix();
    ngl::Mat4 PInv = P.inverse();
    shader->setUniform("PInv",PInv);
    shader->setUniform("texelSizeX",1.0f/_w);
    shader->setUniform("texelSizeY",1.0f/_h);

    (*shader)["BilateralFilter"]->use();
    shader->setUniform("filterRadius",5.0f/width());
    shader->setUniform("texelSize",1.0f/width());
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::timerEvent(QTimerEvent *){
    updateGL();
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::paintGL(){

    // calculate the framerate
    QTime newTime = m_currentTime.currentTime();
    int msecsPassed = m_currentTime.msecsTo(newTime);
    m_currentTime = m_currentTime.currentTime();

    //update our fluid simulation with our time step
    m_SPHEngine->update((float)msecsPassed/1000.0);

    // create the rotation matrices
    ngl::Mat4 rotX;
    ngl::Mat4 rotY;

    rotX.rotateX(m_spinXFace);
    rotY.rotateY(m_spinYFace);
    // multiply the rotations
    m_mouseGlobalTX=rotY*rotX;
    // add the translations
    m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
    m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
    m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


    // Calculate MVP matricies
    ngl::Mat4 P = m_cam->getProjectionMatrix();
    ngl::Mat4 MV = m_mouseGlobalTX * m_cam->getViewMatrix();
    ngl::Mat4 MVP = MV * P;
    ngl::Mat4 Pinv = P.inverse();

    //Here is where we will ultimately load our matricies to shader once written
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["ParticleDepth"]->use();

    //calculate the eyespace radius of our points
    ngl::Vec4 esr(m_pointSize*0.5,0,0,1.0);
    esr = Pinv * esr;
    //std::cout<<"real world size: "<<esr.m_x<<" "<<esr.m_y<<" "<<esr.m_y<<" "<<esr.m_w<<std::endl;

    shader->setUniform("screenWidth",width());
    shader->setUniform("pointSize",m_pointSize);
    shader->setUniform("pointRadius",esr.m_x/esr.m_w);
    shader->setUniform("P",P);
    shader->setUniform("MV",MV);
    shader->setUniform("MVP",MVP);


    // Render to our depth framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthFrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_staticDepthBuffer);
    //clear our frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_SPHEngine->drawArrays();


    // Render to our bilateral filter framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_bilateralFrameBuffer);
    //clear our frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //bind our fluid shader
    (*shader)["BilateralFilter"]->use();
    //set our blur direction
    shader->setUniform("blurDir",1.0f,0.0f);
    shader->setUniform("filterRadius",2.0f/height());
    //bind our billboard and texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,m_depthRender);
    glBindVertexArray(m_billboardVAO);
    glDrawArrays(GL_TRIANGLES,0,6);

    glBindTexture(GL_TEXTURE_2D,m_bilateralRender);
    shader->setUniform("filterRadius",2.0f/width());
    shader->setUniform("blurDir",0.0f,1.0f);
    //chagne our blur direction
    glDrawArrays(GL_TRIANGLES,0,6);

    //unbind our local static frame buffer so we now render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //bind our fluid shader
    (*shader)["FluidShader"]->use();
    //bind our bilateral render texture
    glBindTexture(GL_TEXTURE_2D,m_bilateralRender);
    //draw our billboard
    glDrawArrays(GL_TRIANGLES,0,6);


    //write our framerate
    QString text;
    if(msecsPassed==0){
        text.sprintf("framerate is faster than we can calculate lul :')");
    }
    else{
        text.sprintf("framerate is %f",(float)(1000.0/msecsPassed));
    }
    m_text->renderText(10,20,text);

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::keyPressEvent(QKeyEvent *_event){
    if(_event->key()==Qt::Key_Escape){
        QGuiApplication::exit();
    }
    if(_event->key()==Qt::Key_Plus){
        m_pointSize+=0.5f;
        std::cout<<"particle size: "<<m_pointSize<<std::endl;
    }
    if(_event->key()==Qt::Key_Minus){
        m_pointSize-=0.5f;
        std::cout<<"particle size: "<<m_pointSize<<std::endl;
    }
    if(_event->key()==Qt::Key_E){
        m_SPHEngine->update(0.01);
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mouseMoveEvent (QMouseEvent * _event)
{
    // note the method buttons() is the button state when event was called
    // this is different from button() which is used to check which button was
    // pressed when the mousePress/Release event is generated
    if(m_rotate && _event->buttons() == Qt::LeftButton){
        int diffx=_event->x()-m_origX;
        int diffy=_event->y()-m_origY;
        m_spinXFace += (float) 0.5f * diffy;
        m_spinYFace += (float) 0.5f * diffx;
        m_origX = _event->x();
        m_origY = _event->y();
        updateGL();
    }
    // right mouse translate code
    else if(m_translate && _event->buttons() == Qt::RightButton)
    {
        int diffX = (int)(_event->x() - m_origXPos);
        int diffY = (int)(_event->y() - m_origYPos);
        m_origXPos=_event->x();
        m_origYPos=_event->y();
        m_modelPos.m_x += INCREMENT * diffX;
        m_modelPos.m_y -= INCREMENT * diffY;
        updateGL();
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mousePressEvent ( QMouseEvent * _event)
{
    // this method is called when the mouse button is pressed in this case we
    // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
    if(_event->button() == Qt::LeftButton){
        m_origX = _event->x();
        m_origY = _event->y();
        m_rotate = true;
    }
    // right mouse translate mode
    else if(_event->button() == Qt::RightButton)
    {
        m_origXPos = _event->x();
        m_origYPos = _event->y();
        m_translate=true;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mouseReleaseEvent ( QMouseEvent * _event )
{
    // this event is called when the mouse button is released
    // we then set Rotate to false
    if(_event->button() == Qt::LeftButton){
        m_rotate = false;
    }
    // right mouse translate mode
    if (_event->button() == Qt::RightButton)
    {
        m_translate=false;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::wheelEvent(QWheelEvent *_event)
{
    // check the diff of the wheel position (0 means no change)
    if(_event->delta() > 0)
    {
        m_modelPos.m_z+=ZOOM;
    }
    else if(_event->delta() <0 )
    {
        m_modelPos.m_z-=ZOOM;
    }
    updateGL();
}
//-----------------------------------------------------------------------------------------------------------------------

