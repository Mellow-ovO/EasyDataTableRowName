#include "EasyDataTableRowNameEditor.h"

#define LOCTEXT_NAMESPACE "FEasyDataTableRowNameEditorModule"

void FEasyDataTableRowNameEditorModule::StartupModule()
{
	Identifier = MakeShared<EasyDataTableRowName::Editor::FDataTableRowNamePropertyTypeIdentifier>();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout
		("NameProperty"
		,FOnGetPropertyTypeCustomizationInstance::CreateStatic(&EasyDataTableRowName::Editor::DataTableRowNameCustomization::MakeInstance)
		,Identifier);
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FEasyDataTableRowNameEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("NameProperty",Identifier);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FEasyDataTableRowNameEditorModule, EasyDataTableRowNameEditor)