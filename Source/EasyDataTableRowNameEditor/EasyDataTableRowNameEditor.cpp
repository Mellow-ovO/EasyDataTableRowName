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

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	DataTableNameOptionsCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::MakeInstance));

}

void FEasyDataTableRowNameEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("NameProperty",Identifier);

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	BlueprintEditorModule.UnregisterVariableCustomization(FProperty::StaticClass(), DataTableNameOptionsCustomizationHandle);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FEasyDataTableRowNameEditorModule, EasyDataTableRowNameEditor)