//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/CubeTextureScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "Utils/Image.h"

#define EDGE_LENGTH 10u
#define FACE_SIZE EDGE_LENGTH * EDGE_LENGTH * 4u

namespace ramses_internal
{
    CubeTextureScene::CubeTextureScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(getTestEffect("ramses-test-client-cubeSphere"))
        , m_sphereMesh(nullptr)
        , m_transformNode(nullptr)
    {
        init(static_cast<EState>(state));
    }

    void CubeTextureScene::divideUnitSphereTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, long depth)
    {
        // make sure points are on unit sphere surface
        p1 = glm::normalize(p1);
        p2 = glm::normalize(p2);
        p3 = glm::normalize(p3);

        if (depth == 0)
        {
            // enough recursions, store current positions and indices (no point sharing implemented)
            uint16_t currentPositions = static_cast<uint16_t>(m_spherePositions.size());
            m_sphereIndices.push_back(currentPositions);
            m_sphereIndices.push_back(currentPositions + 1);
            m_sphereIndices.push_back(currentPositions + 2);
            m_spherePositions.push_back(p1);
            m_spherePositions.push_back(p2);
            m_spherePositions.push_back(p3);
            // normals are same as point for unit sphere
            m_sphereNormals.push_back(p1);
            m_sphereNormals.push_back(p2);
            m_sphereNormals.push_back(p3);
        }
        else
        {
            // calculate points halfway between the given points. can use simple addition because of normalization
            glm::vec3 v12 = p1 + p2;
            glm::vec3 v23 = p2 + p3;
            glm::vec3 v31 = p3 + p1;

            // subdivide given triangle into 4 smaller ones
            divideUnitSphereTriangle(p1, v12, v31, depth - 1);
            divideUnitSphereTriangle(p2, v23, v12, depth - 1);
            divideUnitSphereTriangle(p3, v31, v23, depth - 1);
            divideUnitSphereTriangle(v12, v23, v31, depth - 1);
        }
    }

    void CubeTextureScene::initializeUnitSphere()
    {
        // create sphere by recursively subdividing a tetrahedron (pyramid with triangle base) into smaller triangles and project
        // them on unit sphere
        glm::vec3 tetrahedron[] = { glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, -1.f), glm::vec3(-1.f, 1.f, -1.f), glm::vec3(-1.f, -1.f, 1.f) };
        divideUnitSphereTriangle(tetrahedron[1], tetrahedron[2], tetrahedron[0], 5);
        divideUnitSphereTriangle(tetrahedron[2], tetrahedron[3], tetrahedron[0], 5);
        divideUnitSphereTriangle(tetrahedron[3], tetrahedron[1], tetrahedron[0], 5);
        divideUnitSphereTriangle(tetrahedron[1], tetrahedron[3], tetrahedron[2], 5);
    }

    void CubeTextureScene::init(EState state)
    {
        m_transformNode = m_scene.createNode("cubeTransform");
        m_transformNode->setTranslation({0.0f, 0.0f, -8.0f});
        m_sphereMesh = m_scene.createMeshNode("Sphere");
        addMeshNodeToDefaultRenderGroup(*m_sphereMesh);
        m_sphereMesh->setParent(*m_transformNode);

        ramses::Appearance* pAppearance = m_scene.createAppearance(*m_effect, "sphereAppearance");

        ramses::TextureCube* cubeTexture = createTextureCube(state);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Clamp,
            ramses::ETextureAddressMode::Clamp,
            ramses::ETextureSamplingMethod::Linear,
            ramses::ETextureSamplingMethod::Linear,
            *cubeTexture);

        ramses::UniformInput cubeTextureinput;
        m_effect->findUniformInput("cubeTex", cubeTextureinput);
        pAppearance->setInputTexture(cubeTextureinput, *sampler);

        initializeUnitSphere();

        const ramses::ArrayResource* pVertexPositions = m_scene.createArrayResource(static_cast<uint32_t>(m_spherePositions.size()), m_spherePositions.data());
        const ramses::ArrayResource* pVertexNormals = m_scene.createArrayResource(static_cast<uint32_t>(m_sphereNormals.size()), m_sphereNormals.data());
        const ramses::ArrayResource* pIndices = m_scene.createArrayResource(static_cast<uint32_t>(m_sphereIndices.size()), m_sphereIndices.data());

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput normalsInput;
        m_effect->findAttributeInput("a_position", positionsInput);
        m_effect->findAttributeInput("a_normal", normalsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*m_effect, "sphere geometry");
        geometry->setIndices(*pIndices);
        geometry->setInputBuffer(positionsInput, *pVertexPositions);
        geometry->setInputBuffer(normalsInput, *pVertexNormals);

        m_sphereMesh->setAppearance(*pAppearance);
        m_sphereMesh->setGeometryBinding(*geometry);
    }

    ramses::TextureCube* CubeTextureScene::createTextureCube(EState state)
    {
        switch (state)
        {
        case EState_RGBA8:
        {
            ramses_internal::Image imagePX;
            imagePX.loadFromFilePNG("res/ramses-test-client-cube-px.png");
            ramses_internal::Image imageNX;
            imageNX.loadFromFilePNG("res/ramses-test-client-cube-nx.png");
            ramses_internal::Image imagePY;
            imagePY.loadFromFilePNG("res/ramses-test-client-cube-py.png");
            ramses_internal::Image imageNY;
            imageNY.loadFromFilePNG("res/ramses-test-client-cube-ny.png");
            ramses_internal::Image imagePZ;
            imagePZ.loadFromFilePNG("res/ramses-test-client-cube-pz.png");
            ramses_internal::Image imageNZ;
            imageNZ.loadFromFilePNG("res/ramses-test-client-cube-nz.png");

            ramses::CubeMipLevelData mipLevelData(
                static_cast<uint32_t>(imagePX.getData().size()),
                imagePX.getData().data(), imageNX.getData().data(),
                imagePY.getData().data(), imageNY.getData().data(),
                imagePZ.getData().data(), imageNZ.getData().data());

            return m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, imageNY.getWidth(), 1, &mipLevelData, false);
        }
        case EState_BGRA_Swizzled:
        {
            ramses::TextureSwizzle bgraSwizzle = {ramses::ETextureChannelColor::Blue, ramses::ETextureChannelColor::Green, ramses::ETextureChannelColor::Red, ramses::ETextureChannelColor::Alpha};
            ramses_internal::Image imagePX;
            imagePX.loadFromFilePNG("res/ramses-test-client-cube-px.png");
            ramses_internal::Image imageNX;
            imageNX.loadFromFilePNG("res/ramses-test-client-cube-nx.png");
            ramses_internal::Image imagePY;
            imagePY.loadFromFilePNG("res/ramses-test-client-cube-py.png");
            ramses_internal::Image imageNY;
            imageNY.loadFromFilePNG("res/ramses-test-client-cube-ny.png");
            ramses_internal::Image imagePZ;
            imagePZ.loadFromFilePNG("res/ramses-test-client-cube-pz.png");
            ramses_internal::Image imageNZ;
            imageNZ.loadFromFilePNG("res/ramses-test-client-cube-nz.png");

            ramses::CubeMipLevelData mipLevelData(
                static_cast<uint32_t>(imagePX.getData().size()),
                imagePX.getData().data(), imageNX.getData().data(),
                imagePY.getData().data(), imageNY.getData().data(),
                imagePZ.getData().data(), imageNZ.getData().data());

            return m_scene.createTextureCube(ramses::ETextureFormat::RGBA8, imageNY.getWidth(), 1, &mipLevelData, false, bgraSwizzle);
        }
        case EState_Float:
        {
            // 2x2 texture with RGB + white.
            const float texture[] = {1.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f};

            const uint8_t* texturePtr = reinterpret_cast<const uint8_t*>(texture);
            ramses::CubeMipLevelData mipLevelData(sizeof(texture), texturePtr, texturePtr, texturePtr, texturePtr, texturePtr, texturePtr);

            return m_scene.createTextureCube(ramses::ETextureFormat::RGB32F, 2, 1, &mipLevelData, false);
        }
        default:
            assert(!"Invalid texture type");
            return nullptr;
        }
    }
}
