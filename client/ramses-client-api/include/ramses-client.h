//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_CLIENT_H
#define RAMSES_RAMSES_CLIENT_H

/**
 * @defgroup CoreAPI The Ramses Core API
 * This group contains all of the Ramses Core API types.
 */

#include "ramses-client-api/RamsesClient.h"

// Scene
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/IClientEventHandler.h"
#include "ramses-client-api/DataObject.h"

// Effect
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"

// Resources
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"

// Data Buffers
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"

// Iterators
#include "ramses-client-api/SceneIterator.h"
#include "ramses-client-api/RenderPassGroupIterator.h"
#include "ramses-client-api/RenderGroupMeshIterator.h"
#include "ramses-client-api/SceneGraphIterator.h"
#include "ramses-client-api/SceneObjectIterator.h"

#endif
