//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/GeometryInstanceScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/MeshNode.h"
#include <cassert>

namespace ramses_internal
{
    GeometryInstanceScene::GeometryInstanceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = nullptr;
        switch (state)
        {
        case GEOMETRY_INSTANCE_UNIFORM:
            effect = getTestEffect("ramses-test-instancing-uniform");
            break;
        case GEOMETRY_INSTANCE_VERTEX:
        case GEOMETRY_INSTANCE_AND_NOT_INSTANCE:
            effect = getTestEffect("ramses-test-instancing-vertex");
            break;
        default:
            assert(false);
        }

        ramses::Appearance* appearance = m_scene.createAppearance(*effect);

        {
            //create the instanced mesh
            ramses::MeshNode* meshNode = m_scene.createMeshNode("instanced_mesh");
            meshNode->setInstanceCount(NumInstances);
            addMeshNodeToDefaultRenderGroup(*meshNode, 0);
            meshNode->setAppearance(*appearance);

            ramses::GeometryBinding* geometry = createGeometry(*effect);
            if (GEOMETRY_INSTANCE_UNIFORM == state)
            {
                setInstancedUniforms(*appearance);
            }
            else
            {
                setInstancedAttributes(*effect, geometry, 1);
            }
            meshNode->setGeometryBinding(*geometry);
        }

        if (GEOMETRY_INSTANCE_AND_NOT_INSTANCE == state)
        {
            //create the not instanced mesh
            //Instancing is not enabled, so the translation and color buffers will advance per vertex.
            //This means, the vertices of the triangle will be one-one vertex of each individual triangle, with
            //the corresponding color. This will result in a weird looking triangle.
            ramses::MeshNode* notInstancedMeshNode = m_scene.createMeshNode("not_instanced_mesh");
            addMeshNodeToDefaultRenderGroup(*notInstancedMeshNode, 1);
            notInstancedMeshNode->setAppearance(*appearance);

            ramses::Node* translateNode = m_scene.createNode();
            translateNode->setTranslation({0.f, -1.5f, 0.f});
            notInstancedMeshNode->setParent(*translateNode);

            ramses::GeometryBinding* notInstancedGeometry = createGeometry(*effect);
            setInstancedAttributes(*effect, notInstancedGeometry, 0);
            notInstancedMeshNode->setGeometryBinding(*notInstancedGeometry);
        }
    }

    void GeometryInstanceScene::setInstancedUniforms(ramses::Appearance& appearance)
    {
        const ramses::Effect& effect = appearance.getEffect();

        ramses::UniformInput translationInput;
        effect.findUniformInput("translation", translationInput);

        ramses::UniformInput colorInput;
        effect.findUniformInput("color", colorInput);

        static const std::array<ramses::vec3f, NumInstances> translation =
        {
            ramses::vec3f{ -0.5f,  0.5f, 0.0f },
            ramses::vec3f{ 0.0f,   0.0f, 1.0f },
            ramses::vec3f{ 0.5f,  -0.5f, 2.0f }
        };

        static const std::array<ramses::vec4f, NumInstances> color =
        {
            ramses::vec4f{ 0.3f, 0.3f, 0.3f, 1.0f },
            ramses::vec4f{ 1.0f, 0.0f, 0.0f, 1.0f },
            ramses::vec4f{ 0.1f, 1.0f, 0.1f, 1.0f }
        };

        appearance.setInputValue(translationInput, NumInstances, translation.data());
        appearance.setInputValue(colorInput, NumInstances, color.data());
    }

    void GeometryInstanceScene::setInstancedAttributes(const ramses::Effect& effect, ramses::GeometryBinding* geometry, uint32_t instancingDivisor)
    {
        ramses::AttributeInput translationInput;
        effect.findAttributeInput("translation", translationInput);

        ramses::AttributeInput colorInput;
        effect.findAttributeInput("color", colorInput);

        static const std::array<ramses::vec3f, 3u> translation
        {
            ramses::vec3f{ -0.5f,  0.5f, 0.0f },
            ramses::vec3f{ 0.0f,   0.0f, 1.0f },
            ramses::vec3f{ 0.5f,  -0.5f, 2.0f }
        };

        static const std::array<ramses::vec4f, 3u> color
        {
            ramses::vec4f{ 0.3f, 0.3f, 0.3f, 1.0f },
            ramses::vec4f{ 1.0f, 0.0f, 0.0f, 1.0f },
            ramses::vec4f{ 0.1f, 1.0f, 0.1f, 1.0f }
        };

        const ramses::ArrayResource* translationsArray = m_scene.createArrayResource(3u, translation.data());
        geometry->setInputBuffer(translationInput, *translationsArray, instancingDivisor);
        const ramses::ArrayResource* colorArray = m_scene.createArrayResource(3u, color.data());
        geometry->setInputBuffer(colorInput, *colorArray, instancingDivisor);
    }

    ramses::GeometryBinding* GeometryInstanceScene::createGeometry(const ramses::Effect& effect)
    {
        static const std::array<ramses::vec3f, 3u> verticesData
        {
            ramses::vec3f{ -1.0f,  0.f,  -15.0f },
            ramses::vec3f{ 1.0f, 0.f,  -15.0f },
            ramses::vec3f{ 0.0f, 1.f,  -15.0f }
        };

        static const uint16_t indicesData[] =
        {
            0, 1, 2
        };

        const ramses::ArrayResource* indices  = m_scene.createArrayResource(NumInstances, indicesData);
        const ramses::ArrayResource* vertices  = m_scene.createArrayResource(NumInstances, verticesData.data());

        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(effect);

        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertices);

        return geometry;
    }
}
