#pragma once

#include "CoreMinimal.h"
#include "Customization/DataTableRowNameCustomization.h"
#include "Modules/ModuleManager.h"

class FEasyDataTableRowNameEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

protected:

    TSharedPtr<EasyDataTableRowName::Editor::FDataTableRowNamePropertyTypeIdentifier> Identifier;
};
