#pragma once

#include <QtGui/QVector3D>
#include <string>

struct MaterialRGBValue
{
	float R;
	float G;
	float B;
};

class Material
{
public:
    Material(std::string p_name);
    virtual ~Material() = default;

    std::string getName() const
    {
        return m_name;
    }

    MaterialRGBValue getAmbientColor() const
    {
        return m_ambientColor;
    }

    void setAmbientColor(MaterialRGBValue p_rgbValue)
    {
        m_ambientColor = p_rgbValue;
    }

    MaterialRGBValue getDiffuseColor() const
    {
        return m_diffuseColor;
    }

    void setDiffuseColor(MaterialRGBValue p_rgbValue)
    {
        m_diffuseColor = p_rgbValue;
    }

    MaterialRGBValue getSpecularColor() const
    {
        return m_specularColor;
    }

    void setSpecularColor(MaterialRGBValue p_rgbValue)
    {
        m_specularColor = p_rgbValue;
    }

    float getSpecularExponent() const
    {
        return m_specularExponent;
    }

    void setSpecularExponent(float p_specularExponent)
    {
        m_specularExponent = p_specularExponent;
    }

    const std::string& getAmbientTexturePath() const
    {
        return m_ambientTexturePath;
    }

    void setAmbientTexturePath(std::string& p_texturePath)
    {
        m_ambientTexturePath = p_texturePath;
    }

    const std::string& getDiffuseTexturePath() const
    {
        return m_diffuseTexturePath;
    }

    void setDiffuseTexturePath(std::string& p_texturePath)
    {
        m_diffuseTexturePath = p_texturePath;
    }

    const std::string& getSpecularTexturePath() const
    {
        return m_specularTexturePath;
    }

    void setSpecularTexturePath(std::string& p_texturePath)
    {
        m_specularTexturePath = p_texturePath;
    }

private:
    std::string m_name;

    MaterialRGBValue m_ambientColor{ 0.0f, 0.0f, 0.0f };
    MaterialRGBValue m_diffuseColor{ 0.0f, 0.0f, 0.0f };
    MaterialRGBValue m_specularColor{ 0.0f, 0.0f, 0.0f };
    float m_specularExponent{ 20.0f };
    std::string m_ambientTexturePath{ "" };
    std::string m_diffuseTexturePath{ "" };
    std::string m_specularTexturePath{ "" };
};

