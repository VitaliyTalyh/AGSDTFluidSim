//----------------------------------------------------------------------------------------------------------------------
/// @file GLTextureLib.h
/// @class GLTextureLib
/// @author Declan Russell
/// @date 28/04/2015
/// @version 1.0
/// @brief Singleton class for creating and storing textures in a library
//----------------------------------------------------------------------------------------------------------------------


#ifndef GLTEXTURELIB_H
#define GLTEXTURELIB_H

#include <map>
#include <GLTexture.h>
#include <string>

class GLTextureLib
{
public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief add a texture to our texture library
    /// @param _name - the name of our texture
    //----------------------------------------------------------------------------------------------------------------------
    GLTexture * addTexture(std::string _name,GLenum _target, GLint _level, GLint _internalFormat, GLsizei _width, GLsizei _height, GLint _border, GLenum _format, GLenum _type, const GLvoid *_data);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief add a cube map to our texture library
    /// @param _name - the name of our texture
    //----------------------------------------------------------------------------------------------------------------------
    GLTexture * addCubeMap(std::string _name,GLenum _target, GLint _level, GLint _internalFormat, GLsizei _width, GLsizei _height, GLint _border, GLenum _format, GLenum _type, const GLvoid * _front, const GLvoid * _back, const GLvoid * _top, const GLvoid * _bottom, const GLvoid * _left, const GLvoid * _right);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief returns an instance of our texture library
    //----------------------------------------------------------------------------------------------------------------------
    static GLTextureLib *getInstance();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief overloaded operators to access a texture in our library
    //----------------------------------------------------------------------------------------------------------------------
    GLTexture * operator[](const std::string &_name);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief overloaded operators to access a texture in our library
    //----------------------------------------------------------------------------------------------------------------------
    GLTexture * operator[](const char *_name);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our destructor. Removes all textures from library
    //----------------------------------------------------------------------------------------------------------------------
    ~GLTextureLib();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Deletes our library
    //----------------------------------------------------------------------------------------------------------------------
    inline void destroy(){ delete m_instance; }
    //----------------------------------------------------------------------------------------------------------------------
private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our default constructor. Don't want this to be accessable as this is a singleton class
    //----------------------------------------------------------------------------------------------------------------------
    GLTextureLib(){}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the current texture in use
    //----------------------------------------------------------------------------------------------------------------------
    GLTexture *m_currentTexture;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the current texture in use name
    //----------------------------------------------------------------------------------------------------------------------
    std::string m_currentTextureName;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our static instance of our texture library
    //----------------------------------------------------------------------------------------------------------------------
    static GLTextureLib* m_instance;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief mip map for storing our textures
    //----------------------------------------------------------------------------------------------------------------------
    std::map<std::string,GLTexture*> m_textures;
    //----------------------------------------------------------------------------------------------------------------------

};

#endif // GLTEXTURELIB_H
