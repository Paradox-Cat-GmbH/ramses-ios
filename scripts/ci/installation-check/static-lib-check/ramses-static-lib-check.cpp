//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-client.h"
#include "ramses-text.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-renderer-api/IRendererEventHandler.h"

int main(int argc, char* argv[])
{
    printf("Start ramses-static-lib-check\n");
    ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};

    ramses::RamsesFramework framework{frameworkConfig};
    framework.isConnected();

    ramses::RendererConfig config;
    ramses::RamsesRenderer* renderer(framework.createRenderer(config));
    renderer->flush();

    ramses::RamsesClient* ramses(framework.createClient("ramses-static-lib-check"));
    ramses::Scene* scene = ramses->createScene(ramses::sceneId_t(1u));
    scene->isPublished();

    ramses::FontRegistry fontRegistry;

    printf("End ramses-static-lib-check\n");
    return 0;
}
