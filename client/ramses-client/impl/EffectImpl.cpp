//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectImpl.h"
#include "EffectInputImpl.h"
#include "SerializationContext.h"
#include "RamsesClientImpl.h"
#include "Components/ManagedResource.h"
#include "Resource/IResource.h"
#include "Components/ResourceHashUsage.h"
#include "Resource/EffectResource.h"
#include "Collections/StringOutputStream.h"
#include "EffectInputSemanticUtils.h"
#include "AppearanceUtils.h"

namespace ramses
{
    EffectImpl::EffectImpl(ramses_internal::ResourceHashUsage hashUsage, SceneImpl& scene, std::string_view effectname)
        : ResourceImpl(ERamsesObjectType::Effect, std::move(hashUsage), scene, effectname)
    {
    }

    EffectImpl::~EffectImpl()
    {
    }

    void EffectImpl::initializeFromFrameworkData(const ramses_internal::EffectInputInformationVector& uniformInputs, const ramses_internal::EffectInputInformationVector& attributeInputs, std::optional<EDrawMode> geometryShaderInputType)
    {
        m_effectUniformInputs = uniformInputs;
        m_effectAttributeInputs = attributeInputs;
        m_geometryShaderInputType = geometryShaderInputType;
    }

    status_t EffectImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ResourceImpl::serialize(outStream, serializationContext));

        outStream << static_cast<uint32_t>(m_effectUniformInputs.size());
        for(const auto& input : m_effectUniformInputs)
        {
            outStream << input.inputName;
            outStream << static_cast<uint32_t>(input.dataType);
            outStream << static_cast<uint32_t>(input.elementCount);
            outStream << static_cast<uint32_t>(input.semantics);
        }

        outStream << static_cast<uint32_t>(m_effectAttributeInputs.size());
        for (const auto& input : m_effectAttributeInputs)
        {
            outStream << input.inputName;
            outStream << static_cast<uint32_t>(input.dataType);
            outStream << static_cast<uint32_t>(input.semantics);
        }

        const int32_t gsInputType = (m_geometryShaderInputType ? static_cast<int32_t>(*m_geometryShaderInputType) : -1);
        outStream << gsInputType;

        return StatusOK;
    }

    status_t EffectImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ResourceImpl::deserialize(inStream, serializationContext));

        uint32_t count = 0u;
        inStream >> count;
        m_effectUniformInputs.resize(count);
        for (uint32_t i = 0u; i < count; ++i)
        {
            inStream >> m_effectUniformInputs[i].inputName;

            uint32_t dataTypeAsUInt = 0;
            inStream >> dataTypeAsUInt;
            uint32_t elementCount = 0;
            inStream >> elementCount;
            uint32_t semanticAsUInt = 0;
            inStream >> semanticAsUInt;

            m_effectUniformInputs[i].dataType = static_cast<ramses_internal::EDataType>(dataTypeAsUInt);
            m_effectUniformInputs[i].elementCount = elementCount;
            m_effectUniformInputs[i].semantics = static_cast<ramses_internal::EFixedSemantics>(semanticAsUInt);
        }

        inStream >> count;
        m_effectAttributeInputs.resize(count);
        for (uint32_t i = 0u; i < count; ++i)
        {
            inStream >> m_effectAttributeInputs[i].inputName;

            uint32_t dataTypeAsUInt = 0;
            inStream >> dataTypeAsUInt;
            uint32_t semanticAsUInt = 0;
            inStream >> semanticAsUInt;

            m_effectAttributeInputs[i].dataType = static_cast<ramses_internal::EDataType>(dataTypeAsUInt);
            m_effectAttributeInputs[i].elementCount = 1u;
            m_effectAttributeInputs[i].semantics = static_cast<ramses_internal::EFixedSemantics>(semanticAsUInt);
        }

        int32_t gsInputType = -1;
        inStream >> gsInputType;
        if (gsInputType >= 0)
            m_geometryShaderInputType = static_cast<EDrawMode>(gsInputType);

        return StatusOK;
    }

    size_t EffectImpl::getUniformInputCount() const
    {
        return m_effectUniformInputs.size();
    }

    size_t EffectImpl::getAttributeInputCount() const
    {
        return m_effectAttributeInputs.size();
    }

    status_t EffectImpl::getUniformInput(size_t index, EffectInputImpl& inputImpl) const
    {
        if (index >= getUniformInputCount())
        {
            return addErrorEntry("Effect: getUniformInput failed, index out of range!");
        }

        initializeEffectInputData(inputImpl, m_effectUniformInputs[index], index);

        return StatusOK;
    }

    status_t EffectImpl::findUniformInput(EEffectUniformSemantic uniformSemantic, EffectInputImpl& inputImpl) const
    {
        const size_t index = findEffectInputIndex(m_effectUniformInputs, EffectInputSemanticUtils::GetEffectInputSemanticInternal(uniformSemantic));
        if (index == InvalidInputIndex)
        {
            return addErrorEntry("Effect: getUniformInput failed, semantic is not defined in effect!");
        }

        const ramses_internal::EffectInputInformation& effectInputInfo = m_effectUniformInputs[index];
        initializeEffectInputData(inputImpl, effectInputInfo, index);

        return StatusOK;
    }

    status_t EffectImpl::getAttributeInput(size_t index, EffectInputImpl& inputImpl) const
    {
        if (index >= getAttributeInputCount())
        {
            return addErrorEntry("Effect: getAttributeInput failed, index out of range!");
        }

        initializeEffectInputData(inputImpl, m_effectAttributeInputs[index], index);

        return StatusOK;
    }

    status_t EffectImpl::findAttributeInput(EEffectAttributeSemantic attributeSemantic, EffectInputImpl& inputImpl) const
    {
        const size_t index = findEffectInputIndex(m_effectAttributeInputs, EffectInputSemanticUtils::GetEffectInputSemanticInternal(attributeSemantic));
        if (index == InvalidInputIndex)
        {
            return addErrorEntry("Effect: getAttributeInput failed, semantic not defined in effect!");
        }

        const ramses_internal::EffectInputInformation& effectInputInfo = m_effectAttributeInputs[index];
        initializeEffectInputData(inputImpl, effectInputInfo, index);

        return StatusOK;
    }

    status_t EffectImpl::findUniformInput(std::string_view inputName, EffectInputImpl& inputImpl) const
    {
        const size_t index = getEffectInputIndex(m_effectUniformInputs, inputName);
        if (index == InvalidInputIndex)
            return addErrorEntry((ramses_internal::StringOutputStream() << "Effect::findUniformInput: failed, uniform input '" << inputName << "' could not be found in effect '" << getName()).c_str());

        const ramses_internal::EffectInputInformation& effectInputInfo = m_effectUniformInputs[index];
        initializeEffectInputData(inputImpl, effectInputInfo, index);

        return StatusOK;
    }

    status_t EffectImpl::findAttributeInput(std::string_view inputName, EffectInputImpl& inputImpl) const
    {
        const size_t index = getEffectInputIndex(m_effectAttributeInputs, inputName);
        if (index == InvalidInputIndex)
            return addErrorEntry((ramses_internal::StringOutputStream() << "Effect::findAttributeInput: failed, attribute input '" << inputName << "' could not be found in effect '" << getName()).c_str());

        const ramses_internal::EffectInputInformation& effectInputInfo = m_effectAttributeInputs[index];
        initializeEffectInputData(inputImpl, effectInputInfo, index);

        return StatusOK;
    }

    const ramses_internal::EffectInputInformationVector& EffectImpl::getUniformInputInformation() const
    {
        return m_effectUniformInputs;
    }

    const ramses_internal::EffectInputInformationVector& EffectImpl::getAttributeInputInformation() const
    {
        return m_effectAttributeInputs;
    }

    size_t EffectImpl::getEffectInputIndex(const ramses_internal::EffectInputInformationVector& effectInputVector, std::string_view inputName) const
    {
        const size_t numInputs = effectInputVector.size();
        for (size_t i = 0u; i < numInputs; ++i)
        {
            const ramses_internal::EffectInputInformation& effectInputInfo = effectInputVector[i];
            if (effectInputInfo.inputName == inputName)
            {
                return i;
            }
        }

        return InvalidInputIndex;
    }

    size_t EffectImpl::findEffectInputIndex(const ramses_internal::EffectInputInformationVector& effectInputVector, ramses_internal::EFixedSemantics inputSemantics) const
    {
        if (ramses_internal::EFixedSemantics::Invalid == inputSemantics)
            return InvalidInputIndex;

        const size_t numInputs = effectInputVector.size();
        for (size_t i = 0u; i < numInputs; ++i)
        {
            const ramses_internal::EffectInputInformation& effectInputInfo = effectInputVector[i];
            if (effectInputInfo.semantics == inputSemantics)
            {
                return i;
            }
        }

        return InvalidInputIndex;
    }

    void EffectImpl::initializeEffectInputData(EffectInputImpl& effectInputImpl, const ramses_internal::EffectInputInformation& effectInputInfo, size_t index) const
    {
        effectInputImpl.initialize(
            getLowlevelResourceHash(),
            effectInputInfo.inputName,
            effectInputInfo.dataType,
            effectInputInfo.semantics,
            effectInputInfo.elementCount,
            index
            );
    }

    bool EffectImpl::hasGeometryShader() const
    {
        return m_geometryShaderInputType.has_value();
    }

    status_t EffectImpl::getGeometryShaderInputType(EDrawMode& inputType) const
    {
        if (!hasGeometryShader())
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Effect::getGeometryShaderInputType: failed, effect '" << getName() << "' has no geometry shader attached to it!").c_str());
        }

        inputType = *m_geometryShaderInputType;
        return StatusOK;
    }
}
