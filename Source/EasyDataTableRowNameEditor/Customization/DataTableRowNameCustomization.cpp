// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTableRowNameCustomization.h"

#include "DataTableEditorUtils.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "K2Node_Variable.h"
#include "PropertyCustomizationHelpers.h"
#include "Selection.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor/EditorWidgets/Public/SAssetDropTarget.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "EasyDataTableRowName"

FString EasyDataTableRowName::Editor::GetDataTableOptionsKey(const FProperty* Property)
{
	const FNameProperty* NameProperty = CastField<FNameProperty>(Property);
	const FProperty* OwnerProperty = Property->GetOwnerProperty();
	if(NameProperty == nullptr)
	{
		return FString();
	}
	
	bool bIsInContainer = false;
	bool bIsInMapContainer = false;
	if(OwnerProperty != nullptr)
	{
		bIsInMapContainer |= CastField<FMapProperty>(OwnerProperty) != nullptr;
		bIsInContainer |= CastField<FArrayProperty>(OwnerProperty) != nullptr;
		bIsInContainer |= CastField<FSetProperty>(OwnerProperty) != nullptr;
	}
	
	FString DataTablePath;
	if(bIsInMapContainer)
	{
		const FMapProperty* MapProperty = CastField<FMapProperty>(OwnerProperty);
		if (MapProperty->HasMetaData(MD_KeyOptionsFromDataTable) && MapProperty->GetKeyProperty() == Property)
		{
			DataTablePath = OwnerProperty->GetMetaData(MD_KeyOptionsFromDataTable);
		}

		if (MapProperty->HasMetaData(MD_ValueOptionsFromDataTable) && MapProperty->GetValueProperty() == Property)
		{
			DataTablePath = OwnerProperty->GetMetaData(MD_ValueOptionsFromDataTable);
		}
	}
	else
	{
		DataTablePath = bIsInContainer ? OwnerProperty->GetMetaData(MD_OptionsFromDataTable) : NameProperty->GetMetaData(MD_OptionsFromDataTable);
	}
	return DataTablePath;
}

bool EasyDataTableRowName::Editor::FDataTableRowNamePropertyTypeIdentifier::IsPropertyTypeCustomized(
	const IPropertyHandle& InPropertyHandle) const
{
	FProperty* Property = InPropertyHandle.GetProperty();
	FString DataTablePath = GetDataTableOptionsKey(Property);
	
	if(DataTablePath.IsEmpty())
	{
		return false;
	}
	
	const UDataTable* DataTable = FindObject<UDataTable>(nullptr,*DataTablePath);
	return ::IsValid(DataTable);
}

TSharedRef<IPropertyTypeCustomization> EasyDataTableRowName::Editor::DataTableRowNameCustomization::MakeInstance()
{
	return MakeShareable(new DataTableRowNameCustomization());
}

void EasyDataTableRowName::Editor::DataTableRowNameCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	NamePropertyHandle = PropertyHandle;
	// 在使用句柄之前，检查句柄是否为空
	check(NamePropertyHandle.IsValid());
	
	AllRowNames = GetAllRowNames();

	SAssignNew(ComboButton, SComboButton)
		.OnComboBoxOpened(this, &DataTableRowNameCustomization::OnComboBoxOpened)
		.OnGetMenuContent(this, &DataTableRowNameCustomization::GeneratePropertyList)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.ButtonContent()
		[
		   SNew(STextBlock)
		   .Text(this, &DataTableRowNameCustomization::GetSelectedPropertyName)
		];
	
	HeaderRow.NameContent()[PropertyHandle->CreatePropertyNameWidget()]
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.VAlign(EVerticalAlignment::VAlign_Fill)
		.FillWidth(1.0)
		.Padding(5.0)
		[
			ComboButton.ToSharedRef()
		]
		+ SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.VAlign(EVerticalAlignment::VAlign_Fill)
		.Padding(5.0)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeBrowseButton(FSimpleDelegate::CreateSP(this, &DataTableRowNameCustomization::OnBrowseTo))
		]
		
	];
}

void EasyDataTableRowName::Editor::DataTableRowNameCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	
}

TArray<TSharedPtr<FName>> EasyDataTableRowName::Editor::DataTableRowNameCustomization::GetAllRowNames()
{
	TArray<TSharedPtr<FName>> OutNames;
	OutNames.Add(MakeShared<FName>(NAME_None));
	const UDataTable* DataTable = GetOriginDataTable();
	if(!IsValid(DataTable))
	{
		return OutNames;
	}
	for (const FName& RowName : DataTable->GetRowNames())
	{
		OutNames.Add(MakeShared<FName>(RowName));
	}
	
	return OutNames;
}

FText EasyDataTableRowName::Editor::DataTableRowNameCustomization::GetSelectedPropertyName() const
{
	FName CurrentSelectedValue;
	NamePropertyHandle->GetValue(CurrentSelectedValue);

	return FText::FromName(CurrentSelectedValue);
}

TSharedRef<SWidget> EasyDataTableRowName::Editor::DataTableRowNameCustomization::GeneratePropertyList()
{
	return SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			.Visibility(EVisibility::SelfHitTestInvisible)
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.AutoHeight()
			.Padding(5.0)
			[
				SNew(SSearchBox)
				.OnTextChanged(this,&DataTableRowNameCustomization::OnSearchTextChanged)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				SAssignNew(ComboContainer,SListView<TSharedPtr<FName>>)
				.Visibility(EVisibility::Visible)
				.ListItemsSource(&AllRowNames)
				.OnGenerateRow(this, &DataTableRowNameCustomization::OnGenerateRowForPropertyName)
				.OnSelectionChanged(this, &DataTableRowNameCustomization::OnPropertyNameSelectionChanged)
			]
		];
}

void EasyDataTableRowName::Editor::DataTableRowNameCustomization::OnPropertyNameSelectionChanged(TSharedPtr<FName> Item,
	ESelectInfo::Type SelectInfo)
{
	SelectedPropertyName = *Item.Get();
	NamePropertyHandle->SetValue(SelectedPropertyName);
	ComboButton->SetIsOpen(false);
}

TSharedRef<ITableRow> EasyDataTableRowName::Editor::DataTableRowNameCustomization::OnGenerateRowForPropertyName(
	TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<ITableRow> ReturnRow = SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
	   [
		  SNew(STextBlock)
		  .Text(FText::FromName(*Item.Get()))
	   ];

	return ReturnRow;
}

void EasyDataTableRowName::Editor::DataTableRowNameCustomization::OnBrowseTo()
{
	UDataTable* DataTable = GetOriginDataTable();
	if(IsValid(DataTable))
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(DataTable);
		FName CurrentSelectedValue;
		NamePropertyHandle->GetValue(CurrentSelectedValue);
		if(DataTable->GetRowMap().Contains(CurrentSelectedValue))
		{
			FDataTableEditorUtils::SelectRow(DataTable,CurrentSelectedValue);
		}
	}
}

void EasyDataTableRowName::Editor::DataTableRowNameCustomization::OnSearchTextChanged(const FText& Text)
{
	if(!ComboContainer.IsValid())
	{
		return;
	}
	FString SearchString = Text.ToString();
	if(SearchString.Len() > 0)
	{
		AllRowNames = GetAllRowNames();
		AllRowNames = AllRowNames.FilterByPredicate([&SearchString](TSharedPtr<FName> Item)
		{
			return Item.Get()->ToString().Find(SearchString) != INDEX_NONE;
		});
	}
	else
	{
		AllRowNames = GetAllRowNames();
	}
	ComboContainer->RebuildList();
}

void EasyDataTableRowName::Editor::DataTableRowNameCustomization::OnComboBoxOpened()
{
	AllRowNames = GetAllRowNames();
}

UDataTable* EasyDataTableRowName::Editor::DataTableRowNameCustomization::GetOriginDataTable()
{
	FProperty* Property = NamePropertyHandle->GetProperty();
	FString DataTablePath = GetDataTableOptionsKey(Property);
	
	if(DataTablePath.IsEmpty())
	{
		return nullptr;
	}
	
	return FindObject<UDataTable>(nullptr,*DataTablePath);
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::Construct(const FArguments& InArgs)
{
	TSharedPtr<SHorizontalBox> ValueContentBox = nullptr;
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	CachedAssetData = AssetRegistry.GetAssetByObjectPath(InArgs._CachedPath);
	OnPathChanged = InArgs._OnPathChanged;
	ChildSlot
	[
		SNew( SAssetDropTarget )
		.OnAreAssetsAcceptableForDropWithReason( this, &SDataTablePathSelector::OnAssetDraggedOver )
		.OnAssetsDropped( this, &SDataTablePathSelector::OnAssetDropped )
		[
			SAssignNew( ValueContentBox, SHorizontalBox )	
		]
	];

	AssetComboButton = SNew(SComboButton)
		.OnGetMenuContent( this, &SDataTablePathSelector::OnGetMenuContent )
		.OnMenuOpenChanged( this, &SDataTablePathSelector::OnMenuOpenChanged )
		.IsEnabled(true)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Font(FAppStyle::GetFontStyle(PropertyFontStyle))
			.Text(this,&SDataTablePathSelector::OnGetAssetName)
		];
	TSharedPtr<SWidget> ButtonBoxWrapper;
	TSharedRef<SHorizontalBox> ButtonBox = SNew( SHorizontalBox );
	
	TSharedPtr<SHorizontalBox> CustomContentBox;
	ValueContentBox->AddSlot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.VAlign( VAlign_Center )
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()
				[
					AssetComboButton.ToSharedRef()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(ButtonBoxWrapper, SBox)
					.Padding(FMargin(4.0f,0.0f))
					[
						ButtonBox
					]
				]
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SAssignNew(CustomContentBox, SHorizontalBox)
			]
		];

	ButtonBox->AddSlot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding( 2.0f, 0.0f )
		[
			PropertyCustomizationHelpers::MakeUseSelectedButton( FSimpleDelegate::CreateSP( this, &SDataTablePathSelector::OnUse ))
		];


	ButtonBox->AddSlot()
		.Padding( 2.0f, 0.0f )
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			PropertyCustomizationHelpers::MakeBrowseButton(FSimpleDelegate::CreateSP( this, &SDataTablePathSelector::OnBrowse ))
		];

	ButtonBox->AddSlot()
		.Padding( 2.0f, 0.0f )
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			PropertyCustomizationHelpers::MakeClearButton(FSimpleDelegate::CreateSP( this, &SDataTablePathSelector::OnClear ))
		];

	

	NumButtons = ButtonBox->NumSlots();
	
	if (ButtonBoxWrapper.IsValid())
	{
		ButtonBoxWrapper->SetVisibility(NumButtons > 0 ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

bool EasyDataTableRowName::Editor::SDataTablePathSelector::OnAssetDraggedOver(TArrayView<FAssetData> InAssets,
	FText& OutReason) const
{
	if(InAssets.IsEmpty())
	{
		return false;
	}
	UObject* AssetObject = InAssets[0].GetAsset();
	if(!IsValid(AssetObject))
	{
		return false;
	}
	return AssetObject->IsA(UDataTable::StaticClass());
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::OnAssetDropped(const FDragDropEvent&,
	TArrayView<FAssetData> InAssets)
{
	UDataTable* AssetObject = Cast<UDataTable>(InAssets[0].GetAsset());
	if(IsValid(AssetObject))
	{
		SetValue(InAssets[0]);
	}
}

TSharedRef<SWidget> EasyDataTableRowName::Editor::SDataTablePathSelector::OnGetMenuContent()
{
	TArray<const UClass*> AllowedClassList;
	TArray<UFactory*> Factory;
	AllowedClassList.Add(UDataTable::StaticClass());
	return PropertyCustomizationHelpers::MakeAssetPickerWithMenu(CachedAssetData,
																	 true,
																	 AllowedClassList,
																	 Factory,
																	 FOnShouldFilterAsset::CreateSP(this,&SDataTablePathSelector::OnShouldFilterAsset),
																	 FOnAssetSelected::CreateSP(this, &SDataTablePathSelector::OnAssetSelected),
																	 FSimpleDelegate::CreateSP(this, &SDataTablePathSelector::CloseComboButton)
																);
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::OnMenuOpenChanged(bool bOpen)
{
	if (bOpen == false)
	{
		AssetComboButton->SetMenuContent(SNullWidget::NullWidget);
	}
}

bool EasyDataTableRowName::Editor::SDataTablePathSelector::OnShouldFilterAsset(const FAssetData& AssetData)
{
	return false;
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::OnAssetSelected(const FAssetData& AssetData)
{
	SetValue(AssetData);
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::CloseComboButton()
{
	AssetComboButton->SetIsOpen(false);
}

FText EasyDataTableRowName::Editor::SDataTablePathSelector::OnGetAssetName() const
{
	return FText::FromName(CachedAssetData.AssetName);
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::OnUse()
{
	// Load selected assets
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	// try to get a selected object of our class
	const UObject* Selection = nullptr;
	
	Selection = GEditor->GetSelectedObjects()->GetTop( UDataTable::StaticClass() );

	if( Selection )
	{
		SetValue( Selection );
	}
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::OnBrowse()
{
	TArray<FAssetData> AssetDataList;
	AssetDataList.Add( CachedAssetData );
	GEditor->SyncBrowserToObjects( AssetDataList );
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::OnClear()
{
	SetValue(FAssetData());
}

void EasyDataTableRowName::Editor::SDataTablePathSelector::SetValue(const FAssetData& AssetData)
{
	UObject* AssetObject = AssetData.GetAsset();
	if(!IsValid(AssetObject))
	{
		CachedAssetData = FAssetData();
		OnPathChanged.ExecuteIfBound(FSoftObjectPath());
		return;
	}
	if(AssetObject->IsA(UDataTable::StaticClass()))
	{
		AssetComboButton->SetIsOpen(false);
		CachedAssetData = AssetData;
		OnPathChanged.ExecuteIfBound(AssetData.GetSoftObjectPath());
	}
	
}

TSharedPtr<IDetailCustomization> EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::MakeInstance(
	TSharedPtr<IBlueprintEditor> InBlueprintEditor)
{
	const TArray<UObject*>* Objects = (InBlueprintEditor.IsValid() ? InBlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr);
	if (Objects && Objects->Num() == 1)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>((*Objects)[0]))
		{
			return MakeShareable(new DataTableRowNameBPEditorCustomization(InBlueprintEditor, Blueprint));
		}
	}
	return nullptr;
}

UK2Node_Variable* EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::EdGraphSelectionAsVar() const
{
	TWeakPtr<FBlueprintEditor> BlueprintEditor = MyBlueprint.Pin()->GetBlueprintEditor();

	if( BlueprintEditor.IsValid() )
	{
		/** Get the currently selected set of nodes */
		FGraphPanelSelectionSet Objects = BlueprintEditor.Pin()->GetSelectedNodes();

		if (Objects.Num() == 1)
		{
			FGraphPanelSelectionSet::TIterator Iter(Objects);
			UObject* Object = *Iter;

			if (Object && Object->IsA<UK2Node_Variable>())
			{
				return Cast<UK2Node_Variable>(Object);
			}
		}
	}
	return nullptr;
}

bool EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::IsALocalVariable(
	FProperty* VariableProperty) const
{
	return VariableProperty && (VariableProperty->GetOwner<UFunction>() != NULL);
}

bool EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::IsAUserVariable(
	FProperty* VariableProperty) const
{
	FObjectProperty* VariableObjProp = VariableProperty ? CastField<FObjectProperty>(VariableProperty) : NULL;

	if (VariableObjProp != NULL && VariableObjProp->PropertyClass != NULL)
	{
		return FBlueprintEditorUtils::IsVariableCreatedByBlueprint(GetBlueprintObj(), VariableObjProp);
	}
	return true;
}

bool EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::IsABlueprintVariable(
	FProperty* VariableProperty) const
{
	UClass* VarSourceClass = VariableProperty ? VariableProperty->GetOwner<UClass>() : NULL;
	if(VarSourceClass)
	{
		return (VarSourceClass->ClassGeneratedBy != NULL);
	}
	return false;
}

FProperty* EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::SelectionAsProperty() const
{
	if (FEdGraphSchemaAction_BlueprintVariableBase* BPVar = MyBlueprint.Pin()->SelectionAsBlueprintVariable())
	{
		return BPVar->GetProperty();
	}
	else if (UK2Node_Variable* GraphVar = EdGraphSelectionAsVar())
	{
		return GraphVar->GetPropertyForVariable();
	}

	return nullptr;
}

FName EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::GetVariableName() const
{
	if (FEdGraphSchemaAction_BlueprintVariableBase* BPVar = MyBlueprint.Pin()->SelectionAsBlueprintVariable())
	{
		return BPVar->GetVariableName();
	}
	else if (UK2Node_Variable* GraphVar = EdGraphSelectionAsVar())
	{
		return GraphVar->GetVarName();
	}

	return NAME_None;
}

void EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::OnPostEditorRefresh()
{
	CachedVariableProperty = SelectionAsProperty();
	CachedVariableName = GetVariableName();
}

UBlueprint* EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::GetPropertyOwnerBlueprint() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();

	// Cache the Blueprint which owns this VariableProperty
	if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(VariableProperty->GetOwnerClass()))
	{
		return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
	}
	return nullptr;
}

EVisibility EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::
GetDataTableRowNameBPEditorVisibility() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();
	// Cache the Blueprint which owns this VariableProperty
	if (VariableProperty && GetPropertyOwnerBlueprint())
	{
		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
		FEdGraphPinType VariablePinType;
		K2Schema->ConvertPropertyToPinType(VariableProperty, VariablePinType);

		const bool bShowPrivacySetting = IsABlueprintVariable(VariableProperty) && IsAUserVariable(VariableProperty) && !IsALocalVariable(VariableProperty);
		FObjectPropertyBase* ObjectProperty  = CastField<FObjectPropertyBase>(VariableProperty);
		if (!bShowPrivacySetting || (K2Schema->FindSetVariableByNameFunction(VariablePinType) == NULL) || ObjectProperty != nullptr)
		{
			return EVisibility::Collapsed;
		}
	}
	if(ShouldKeyOptionsShow())
	{
		return EVisibility::Visible;
	}
	if(ShouldValueOptionsShow())
	{
		return EVisibility::Visible;
	}
	if(ShouldNameOptionsShow())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

bool EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::ShouldKeyOptionsShow() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();
	if(FMapProperty* MapProperty = CastField<FMapProperty>(VariableProperty))
	{
		return (CastField<FNameProperty>(MapProperty->GetKeyProperty()) != nullptr);
	}
	return false;
}

bool EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::ShouldValueOptionsShow() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();
	if(FMapProperty* MapProperty = CastField<FMapProperty>(VariableProperty))
	{
		return (CastField<FNameProperty>(MapProperty->GetValueProperty()) != nullptr);
	}
	return false;
}

bool EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::ShouldNameOptionsShow() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();
	if(CastField<FNameProperty>(VariableProperty))
	{
		return true;
	}
	
	if(FSetProperty* SetProperty = CastField<FSetProperty>(VariableProperty))
	{
		if(CastField<FNameProperty>(SetProperty->GetElementProperty()))
		{
			return true;
		}
	}
	
	if(FArrayProperty* ArrayProperty = CastField<FArrayProperty>(VariableProperty))
	{
		if(CastField<FNameProperty>(ArrayProperty->Inner))
		{
			return true;
		}
	}
	
	return false;
}

void EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::OnKeyPathChanged(const FSoftObjectPath& Path)
{
	const FName VarName = CachedVariableName;
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(GetBlueprintObj(), VarName, nullptr, MD_KeyOptionsFromDataTable, Path.ToString());
}

EVisibility EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::KeyPathSelectorVisibility() const
{
	if(ShouldKeyOptionsShow())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::ValuePathSelectorVisibility() const
{
	if(ShouldValueOptionsShow())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::CommonPathSelectorVisibility() const
{
	if(ShouldNameOptionsShow())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

void EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::OnValuePathChanged(
	const FSoftObjectPath& Path)
{
	const FName VarName = CachedVariableName;
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(GetBlueprintObj(), VarName, nullptr, MD_ValueOptionsFromDataTable, Path.ToString());

}

void EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::OnCommonPathChanged(
	const FSoftObjectPath& Path)
{
	const FName VarName = CachedVariableName;
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(GetBlueprintObj(), VarName, nullptr, MD_OptionsFromDataTable, Path.ToString());
}

void EasyDataTableRowName::Editor::DataTableRowNameBPEditorCustomization::CustomizeDetails(
	IDetailLayoutBuilder& DetailBuilder)
{
	CachedVariableProperty = SelectionAsProperty();

	if(!CachedVariableProperty.IsValid())
	{
		return;
	}

	CachedVariableName = GetVariableName();
	
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Name Options From Datatable",FText::GetEmpty(),ECategoryPriority::TypeSpecific);
	Category.SetCategoryVisibility(GetDataTableRowNameBPEditorVisibility() == EVisibility::Visible);
	
	FProperty* VariableProperty = CachedVariableProperty.Get();

	Category.AddCustomRow(LOCTEXT("EasyDataTableRowName", "Set Key Options DataTable"))
		.NameContent()
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("EasyDataTableRowName", "Set Key Options DataTable"))
			.Text( LOCTEXT("EasyDataTableRowName", "Set Key Options DataTable"))
			.Font( IDetailLayoutBuilder::GetDetailFontBold() )
		]
		.ValueContent()
		[
			SAssignNew(KeyDataTableSelector,SDataTablePathSelector)
			.CachedPath(FSoftObjectPath(CachedVariableProperty->GetMetaData(MD_KeyOptionsFromDataTable)))
			.OnPathChanged(FOnPathChanged::CreateSP(this,&DataTableRowNameBPEditorCustomization::OnKeyPathChanged))
		]
		.Visibility(TAttribute<EVisibility>(this, &DataTableRowNameBPEditorCustomization::KeyPathSelectorVisibility));

	Category.AddCustomRow(LOCTEXT("EasyDataTableRowName", "Set Value Options DataTable"))
		.NameContent()
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("EasyDataTableRowName", "Set Value Options DataTable"))
			.Text( LOCTEXT("EasyDataTableRowName", "Set Value Options DataTable"))
			.Font( IDetailLayoutBuilder::GetDetailFontBold() )
		]
		.ValueContent()
		[
			SAssignNew(ValueDataTableSelector,SDataTablePathSelector)
			.CachedPath(FSoftObjectPath(CachedVariableProperty->GetMetaData(MD_ValueOptionsFromDataTable)))
			.OnPathChanged(FOnPathChanged::CreateSP(this,&DataTableRowNameBPEditorCustomization::OnValuePathChanged))
		]
		.Visibility(TAttribute<EVisibility>(this, &DataTableRowNameBPEditorCustomization::ValuePathSelectorVisibility));

	Category.AddCustomRow(LOCTEXT("EasyDataTableRowName", "Set Common Options DataTable"))
		.NameContent()
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("EasyDataTableRowName", "Set Common Options DataTable"))
			.Text( LOCTEXT("EasyDataTableRowName", "Set Common Options DataTable"))
			.Font( IDetailLayoutBuilder::GetDetailFontBold() )
		]
		.ValueContent()
		[
			SAssignNew(CommonDataTableSelector,SDataTablePathSelector)
			.CachedPath(FSoftObjectPath(CachedVariableProperty->GetMetaData(MD_OptionsFromDataTable)))
			.OnPathChanged(FOnPathChanged::CreateSP(this,&DataTableRowNameBPEditorCustomization::OnCommonPathChanged))
		]
		.Visibility(TAttribute<EVisibility>(this, &DataTableRowNameBPEditorCustomization::CommonPathSelectorVisibility));

	
}

#undef LOCTEXT_NAMESPACE