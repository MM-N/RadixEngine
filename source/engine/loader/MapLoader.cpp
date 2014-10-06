#include "MapLoader.hpp"

#include <tinyxml.h>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "Player.hpp"
#include "Scene.hpp"
#include "engine/env/Environment.hpp"
#include "engine/Light.hpp"
#include "engine/Mesh.hpp"
#include "MeshLoader.hpp"
#include "XmlHelper.hpp"
#include "engine/Texture.hpp"
#include "TextureLoader.hpp"
#include "engine/Trigger.hpp"
#include "util/math/Vector3f.hpp"

using namespace std;

namespace glPortal {
/** \class MapLoader
 *  Load a map in GlPortal XML format.
 */

  Scene* MapLoader::scene;
  TiXmlHandle MapLoader::rootHandle = TiXmlHandle(0);

/**
 * Get a scene from a map file in XML format.
 */
Scene* MapLoader::getScene(std::string path) {
  scene = new Scene();

  TiXmlDocument doc(string(Environment::getDataDir() + path));
  bool loaded = doc.LoadFile();

  if (loaded) {
    TiXmlHandle docHandle(&doc);
    TiXmlElement* element;

    element = docHandle.FirstChildElement().Element();
    rootHandle = TiXmlHandle(element);

    extractSpawn();
    extractDoor();
    extractModels();
    extractLights();
    extractWalls();
    extractTriggers();
    cout << "File loaded." << endl;
  } else {
    cout << "Unable to load file. " << endl;
    cout << string(Environment::getDataDir()) << path << endl;
  }
  return scene;
}

/**
 * Extract a spawn element containing its rotation and position elements
 */
void MapLoader::extractSpawn() {
  TiXmlElement* spawnElement;
  spawnElement = rootHandle.FirstChild("spawn").Element();

  if (spawnElement) {
    XmlHelper::extractPositionAndRotation(spawnElement, scene->player);
  } else {
    throw std::runtime_error("No spawn position defined.");
  }
}

/**
 * Extract a light elements containing position (x, y, z) and colour (r, g, b) attributes
 */
void MapLoader::extractLights() {
  Vector3f lightPos;
  Vector3f lightColor;
  TiXmlElement* lightElement;
  lightElement = rootHandle.FirstChild("light").Element();

  do {
    XmlHelper::pushAttributeVertexToVector(lightElement, lightPos);

    lightElement->QueryFloatAttribute("r", &lightColor.x);
    lightElement->QueryFloatAttribute("g", &lightColor.y);
    lightElement->QueryFloatAttribute("b", &lightColor.z);
    Light light;
    light.position.set(lightPos.x, lightPos.y, lightPos.z);
    light.color.set(lightColor.x, lightColor.y, lightColor.z);
    scene->lights.push_back(light);
  } while ((lightElement = lightElement->NextSiblingElement("light")) != NULL);
}

void MapLoader::extractDoor() {
  TiXmlElement* endElement;
  endElement = rootHandle.FirstChild("end").Element();

  if (endElement) {
    Entity door;
    XmlHelper::extractPositionAndRotation(endElement, door);
    door.texture = TextureLoader::getTexture("Door.png");
    door.mesh = MeshLoader::getMesh("Door.obj");
    scene->end = door;
  } else {
    throw std::runtime_error("No end position defined.");
  }
}

void MapLoader::extractWalls() {
  TiXmlElement* textureElement = rootHandle.FirstChild("texture").Element();
  string texturePath("none");
  string surfaceType("none");

  if (textureElement) {
    do {
      textureElement->QueryStringAttribute("source", &texturePath);
      textureElement->QueryStringAttribute("type", &surfaceType);
      TiXmlElement* wallBoxElement = textureElement->FirstChildElement("wall");

      if (wallBoxElement) {
        do {
          TiXmlElement* boxPositionElement;
          TiXmlElement* boxScaleElement;

          Entity wall;
          boxPositionElement = wallBoxElement->FirstChildElement("position");
          XmlHelper::pushAttributeVertexToVector(boxPositionElement, wall.position);

          boxScaleElement = wallBoxElement->FirstChildElement("scale");
          XmlHelper::pushAttributeVertexToVector(boxScaleElement, wall.scale);

          wall.texture = TextureLoader::getTexture(texturePath);
          wall.texture.xTiling = 0.5f;
          wall.texture.yTiling = 0.5f;
          wall.mesh = MeshLoader::getPortalBox(wall);
          scene->walls.push_back(wall);
        } while ((wallBoxElement = wallBoxElement->NextSiblingElement("wall")) != NULL);
      }

      texturePath = std::string("none");
    } while ((textureElement = textureElement->NextSiblingElement("texture")) != NULL);
  }
}

void MapLoader::extractTriggers() {
  TiXmlElement* triggerElement = rootHandle.FirstChild("trigger").Element();
  string triggerType("none");

  if (triggerElement) {
    do {
      TiXmlElement* triggerTypeElement;

      Trigger trigger;

      if (triggerElement) {
        triggerElement->QueryStringAttribute("type", &trigger.type);
      }
      
      if (triggerType == "none") {
        throw std::runtime_error("Trigger must define a type attribute.");
      }

      XmlHelper::pushAttributeVertexToVector(triggerElement->FirstChildElement("position"),
                                             trigger.position);
      XmlHelper::pushAttributeVertexToVector(triggerElement->FirstChildElement("scale"),
                                             trigger.scale);
      trigger.texture = TextureLoader::getTexture("redBox.png");
      trigger.mesh = MeshLoader::getPortalBox(trigger);
      scene->triggers.push_back(trigger);

    } while ((triggerElement = triggerElement->NextSiblingElement()) != NULL);
  }
}

void MapLoader::extractModels() {
  Vector3f modelPos;
  string texture("none");
  string mesh("none");
  TiXmlElement* modelElement = rootHandle.FirstChild("model").Element();
  if (modelElement){
    do {
      modelElement->QueryStringAttribute("texture", &texture);
      modelElement->QueryStringAttribute("mesh", &mesh);
      XmlHelper::pushAttributeVertexToVector(modelElement, modelPos);

      Entity model;
      XmlHelper::extractPositionAndRotation(modelElement, model);
      model.texture = TextureLoader::getTexture(texture);
      model.mesh = MeshLoader::getMesh(mesh);
      scene->models.push_back(model);
    } while ((modelElement = modelElement->NextSiblingElement("model")) != NULL);
  }
}
} /* namespace glPortal */
