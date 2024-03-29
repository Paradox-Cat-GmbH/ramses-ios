//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLSLTOEFFECTCONVERTER_H
#define RAMSES_GLSLTOEFFECTCONVERTER_H

#include "glslEffectBlock/GLSlang.h"
#include "Collections/StringOutputStream.h"
#include "Collections/HashMap.h"
#include "SceneAPI/EFixedSemantics.h"
#include "Resource/EffectInputInformation.h"
#include "SceneAPI/RenderState.h"
#include <optional>

#include <string>
#include <string_view>

namespace ramses_internal
{
    class GlslToEffectConverter
    {
    public:
        explicit GlslToEffectConverter(const HashMap<std::string, EFixedSemantics>& semanticInputs);
        ~GlslToEffectConverter();

        bool parseShaderProgram(glslang::TProgram* program);
        std::string getStatusMessage() const;

        const EffectInputInformationVector& getUniformInputs() const;
        const EffectInputInformationVector& getAttributeInputs() const;

        std::optional<EDrawMode> getGeometryShaderInputType() const;

    private:
        enum class EShaderStage
        {
            Vertex = 1,
            Geometry = 2,
            Fragment = 4,
            Invalid
        };

        HashMap<std::string, EFixedSemantics> m_semanticInputs;
        mutable StringOutputStream m_message;

        EffectInputInformationVector m_uniformInputs;
        EffectInputInformationVector m_attributeInputs;

        std::optional<EDrawMode> m_geometryShaderInputType;

        bool parseLinkerObjectsForStage(const TIntermNode* node, EShaderStage stage);
        const glslang::TIntermSequence* getLinkerObjectSequence(const TIntermNode* node) const;
        bool handleSymbol(const glslang::TIntermSymbol* symbol, EShaderStage stage);

        bool getElementCountFromType(const glslang::TType& type, std::string_view inputName, uint32_t& elementCount) const;
        bool setInputTypeFromType(const glslang::TType& type, std::string_view inputName, EffectInputInformationVector& outputVector) const;
        bool setInputTypeFromType(const glslang::TType& type, EffectInputInformation& input) const;
        bool replaceVertexAttributeWithBufferVariant();
        bool setSemanticsOnInput(EffectInputInformation& input) const;
        bool makeUniformsUnique();
        bool createEffectInputType(const glslang::TType& type, std::string_view inputName, uint32_t elementCount, EffectInputInformationVector& outputVector) const;
        std::string getStructFieldIdentifier(std::string_view baseName, std::string_view fieldName, const int32_t arrayIndex) const;
    };
}

#endif
