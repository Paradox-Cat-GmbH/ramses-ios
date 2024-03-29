//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATALINKINGTESTS_H
#define RAMSES_DATALINKINGTESTS_H

#include "IRendererTest.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include <string>

class DataLinkingTests : public IRendererTest
{
public:
    void setUpTestCases(RendererTestsFramework& testFramework) final;
    bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    bool renderAndCompareScreenshot(RendererTestsFramework& testFramework, const std::string& expectedImageName, uint32_t testDisplayIdx = 0u, float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel);

    template <typename LINKSCENE>
    void createAndShowDataLinkScenes(RendererTestsFramework& testFramework);

    void createAndShowMultiTypeLinkScenes(RendererTestsFramework& testFramework);

    enum
    {
        TransformationLinkTest_ConsumerNotLinkedToProvider,
        TransformationLinkTest_ConsumerLinkedToProvider,
        TransformationLinkTest_LinkRemoved,
        TransformationLinkTest_LinkOverridesComsumerTransform,
        TransformationLinkTest_ConsumerLinkedToProviderNested,
        TransformationLinkTest_RemovedProviderFromScene,
        TransformationLinkTest_RemovedConsumerFromScene,
        TransformationLinkTest_ConfidenceMultiLinkTest,

        DataLinkTest_NoLinks,
        DataLinkTest_Linked,
        DataLinkTest_LinksRemoved,
        DataLinkTest_ProviderRemoved,
        DataLinkTest_ConsumerRemoved,

        CameraDataLinkTest_NoLinks,
        CameraDataLinkTest_ViewportLinked,
        CameraDataLinkTest_FrustumLinked,

        TextureLinkTest_NoLinks,
        TextureLinkTest_Linked,
        TextureLinkTest_LinksRemoved,
        TextureLinkTest_ProviderSceneUnmapped,
        TextureLinkTest_ProvidedTextureChanges,

        MultiTypeLinkTest_NoLinks,
        MultiTypeLinkTest_Linked
    };

    ramses::sceneId_t m_sceneIdProvider;
    ramses::sceneId_t m_sceneIdProviderConsumer;
    ramses::sceneId_t m_sceneIdConsumer;

    const glm::vec3 m_cameraLow{ -1.f, 2.f, 8.f };
    const glm::vec3 m_cameraHigh{ 1.f, -2.f, 8.f };
    const glm::vec3 m_cameraMid{ 0.f, 0.f, 8.f };
};

#endif
