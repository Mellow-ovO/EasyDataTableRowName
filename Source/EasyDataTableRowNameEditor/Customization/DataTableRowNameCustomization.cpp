// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTableRowNameCustomization.h"

#include "DataTableEditorUtils.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SSearchBox.h"

bool EasyDataTableRowName::Editor::FDataTableRowNamePropertyTypeIdentifier::IsPropertyTypeCustomized(
	const IPropertyHandle& InPropertyHandle) const
{
	FProperty* Property = InPropertyHandle.GetProperty();
	if(FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		FString DataTablePath = NameProperty->GetMetaData(EasyDataTableRowName::Editor::MD_OptionsFromDataTable);
		if(DataTablePath.IsEmpty())
		{
			return false;
		}
		const UDataTable* DataTable = FindObject<UDataTable>(nullptr,*DataTablePath);
		return ::IsValid(DataTable);
	}
	return false;
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
	FString DataTablePath = NamePropertyHandle->GetMetaData(EasyDataTableRowName::Editor::MD_OptionsFromDataTable);
	if(DataTablePath.IsEmpty())
	{
		return nullptr;
	}
	return FindObject<UDataTable>(nullptr,*DataTablePath);
}
