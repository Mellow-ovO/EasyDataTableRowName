#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

namespace EasyDataTableRowName::Editor
{
	static inline FName MD_OptionsFromDataTable = FName("OptionsFromDataTable");
	
	class FDataTableRowNamePropertyTypeIdentifier : public IPropertyTypeIdentifier
	{
	private:
		virtual bool IsPropertyTypeCustomized(const IPropertyHandle& InPropertyHandle) const override;
	};

	class EASYDATATABLEROWNAMEEDITOR_API DataTableRowNameCustomization: public IPropertyTypeCustomization
	{
	public:
		static TSharedRef<IPropertyTypeCustomization> MakeInstance();
		virtual void CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
		virtual void CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;

	private:
		TArray<TSharedPtr<FName>> GetAllRowNames();
	
		FText GetSelectedPropertyName() const;
		TSharedRef<SWidget> GeneratePropertyList();
		void OnPropertyNameSelectionChanged(TSharedPtr<FName> Item,ESelectInfo::Type SelectInfo);
		TSharedRef<ITableRow> OnGenerateRowForPropertyName(TSharedPtr<FName> Item,const TSharedRef<STableViewBase>& OwnerTable);
		void OnBrowseTo();

	protected:
		void OnSearchTextChanged(const FText& Text);
		void OnComboBoxOpened();

	protected:
		UDataTable* GetOriginDataTable();
		
	private:

		TArray<TSharedPtr<FName>> AllRowNames;
		TSharedPtr<SListView<TSharedPtr<FName>>> ComboContainer;
		FName SelectedPropertyName = FName("None");
	
		TSharedPtr<SComboButton> ComboButton;
		TSharedPtr<IPropertyHandle> NamePropertyHandle;
	};
}





