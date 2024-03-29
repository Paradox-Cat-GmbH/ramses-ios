//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTIPLETRIANGLESSCENE_H
#define RAMSES_MULTIPLETRIANGLESSCENE_H

#include "IntegrationScene.h"
#include "Triangle.h"
#include "Line.h"
#include "SceneAPI/Handles.h"
#include "ramses-client-api/MeshNode.h"
#include "MultiTriangleGeometry.h"

namespace ramses_internal
{
    class MultipleTrianglesScene : public IntegrationScene
    {
    public:
        MultipleTrianglesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        void setState(uint32_t state);

        enum
        {
            THREE_TRIANGLES = 0,
            TRIANGLES_REORDERED,
            ADDITIVE_BLENDING,
            SUBTRACTIVE_BLENDING,
            ALPHA_BLENDING,
            BLENDING_CONSTANT,
            BLENDING_DST_COLOR_AND_ALPHA,
            COLOR_MASK,
            CAMERA_TRANSFORMATION,
            FACE_CULLING,
            DEPTH_FUNC,
            DRAW_MODE,
            STENCIL_TEST_1,
            STENCIL_TEST_2,
            STENCIL_TEST_3,
            SCISSOR_TEST,
            MULTIPLE_DISPLAYS,
            SUBIMAGES,
            PERSPECTIVE_CAMERA,
            ORTHOGRAPHIC_CAMERA,
            THREE_TRIANGLES_WITH_SHARED_COLOR,
            THREE_TRIANGLES_WITH_UNSHARED_COLOR,
            EULER_ROTATION_CONVENTIONS,
        };
    private:
        void setGeometries(uint32_t state);
        void setTransformations(uint32_t state);

        ramses::Effect  * m_Effect;

        ramses::MeshNode* m_meshNode1;
        ramses::MeshNode* m_meshNode2;
        ramses::MeshNode* m_meshNode3;
        ramses::MeshNode* m_meshNode4;
        ramses::MeshNode* m_meshNode5;
        ramses::MeshNode* m_meshNode6;
        ramses::MeshNode* m_meshNode7;

        ramses::Triangle m_whiteTriangle;
        ramses::Triangle m_redTriangle;
        ramses::Triangle m_greenTriangle;
        ramses::Triangle m_blueTriangle;
        ramses::Line     m_yellowLine;
        ramses::MultiTriangleGeometry m_whiteQuad;
        ramses::MultiTriangleGeometry m_triangleFan;
        ramses::Line     m_lineStrip;
        ramses::Line     m_linePoints;
        ramses::Triangle m_redTransparentTriangle;
        ramses::Triangle m_greenTransparentTriangle;
        ramses::Triangle m_blueTransparentTriangle;
        ramses::Triangle m_colorMaskRedTriangle;
        ramses::Triangle m_colorMaskGreenTriangle;
        ramses::Triangle m_colorMaskBlueTriangle;
        ramses::Triangle m_CCWTriangle;
        ramses::Triangle m_CWTriangle;
        ramses::Triangle m_CWTriangleCCWIndices;
    };
}

#endif
