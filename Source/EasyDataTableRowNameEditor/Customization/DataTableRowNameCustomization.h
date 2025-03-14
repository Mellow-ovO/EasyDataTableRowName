#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditor.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "SMyBlueprint.h"

namespace EasyDataTableRowName::Editor
{
	static inline FName MD_OptionsFromDataTable = FName("OptionsFromDataTable");
	static inline FName MD_KeyOptionsFromDataTable = FName("KeyOptionsFromDataTable");
	static inline FName MD_ValueOptionsFromDataTable = FName("ValueOptionsFromDataTable");
	static inline FName PropertyFontStyle( TEXT("PropertyWindow.NormalFont") );

	DECLARE_DELEGATE_OneParam(FOnPathChanged, const FSoftObjectPath&);

	
	FString GetDataTableOptionsKey(const FProperty* Property);
	
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


	class SDataTablePathSelector : public SCompoundWidget
	{
		SLATE_BEGIN_ARGS(SDataTablePathSelector)
			{}
			SLATE_ARGUMENT(FSoftObjectPath, CachedPath)
			SLATE_EVENT(FOnPathChanged, OnPathChanged)

		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);

		protected:
			bool OnAssetDraggedOver( TArrayView<FAssetData> InAssets, FText& OutReason ) const;
			void OnAssetDropped( const FDragDropEvent&, TArrayView<FAssetData> InAssets );

			TSharedRef<SWidget> OnGetMenuContent();
			void OnMenuOpenChanged(bool bOpen);
			bool OnShouldFilterAsset( const FAssetData& AssetData );
			void OnAssetSelected( const FAssetData& AssetData );
			void CloseComboButton();

		FText OnGetAssetName() const;

			void OnUse();
			void OnBrowse();
			void OnClear();
			void SetValue( const FAssetData& AssetData );

		protected:
			FOnPathChanged OnPathChanged;
		
		protected:
			TSharedPtr<SComboButton> AssetComboButton;
			mutable FAssetData CachedAssetData;
			int32 NumButtons = 0;
	};

	

	class DataTableRowNameBPEditorCustomization  : public IDetailCustomization
	{
	
		public:
			DataTableRowNameBPEditorCustomization()
			{
				
			}
	
			DataTableRowNameBPEditorCustomization(TSharedPtr<IBlueprintEditor> InBlueprintEditor, UBlueprint* Blueprint)
				: BlueprintEditorPtr(InBlueprintEditor)
				, Blueprint(Blueprint)
			{
				if(FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(InBlueprintEditor.Get()))
				{
					MyBlueprint = BlueprintEditor->GetMyBlueprintWidget();
				}
			}
	
			virtual ~DataTableRowNameBPEditorCustomization() override
			{
				if(MyBlueprint.IsValid())
				{
					// Remove the callback delegate we registered for
					TWeakPtr<FBlueprintEditor> BlueprintEditor = MyBlueprint.Pin()->GetBlueprintEditor();
					if( BlueprintEditor.IsValid() )
					{
						BlueprintEditor.Pin()->OnRefresh().RemoveAll(this);
					}
				}
			}
			
		public:
			static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor);
			
		public:
			UK2Node_Variable* EdGraphSelectionAsVar() const;
			bool IsALocalVariable(FProperty* VariableProperty) const;
			bool IsAUserVariable(FProperty* VariableProperty) const;
			bool IsABlueprintVariable(FProperty* VariableProperty) const;
		
			FProperty* SelectionAsProperty() const;
			FName GetVariableName() const;
			void OnPostEditorRefresh();
			UBlueprint* GetPropertyOwnerBlueprint() const;
			EVisibility GetDataTableRowNameBPEditorVisibility() const;

		protected:
			bool ShouldKeyOptionsShow() const;
			bool ShouldValueOptionsShow() const;
			bool ShouldNameOptionsShow() const;

			void OnKeyPathChanged(const FSoftObjectPath& Path);
			EVisibility KeyPathSelectorVisibility() const;
		
			void OnValuePathChanged(const FSoftObjectPath& Path);
			EVisibility ValuePathSelectorVisibility() const;
		
			void OnCommonPathChanged(const FSoftObjectPath& Path);
			EVisibility CommonPathSelectorVisibility() const;

		public:
			virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
		protected:
			UBlueprint* GetBlueprintObj() const { return Blueprint.Get(); }
	
		protected:

			TWeakPtr<SDataTablePathSelector> KeyDataTableSelector;
			TWeakPtr<SDataTablePathSelector> ValueDataTableSelector;
			TWeakPtr<SDataTablePathSelector> CommonDataTableSelector;
	
			/** The Blueprint editor we are embedded in */
			TWeakPtr<IBlueprintEditor> BlueprintEditorPtr;
	
			
			TWeakObjectPtr<UBlueprint> Blueprint;
			
			/** Cached property for the variable we are affecting */
			TWeakFieldPtr<FProperty> CachedVariableProperty;
	
			/** Cached name for the variable we are affecting */
			FName CachedVariableName;
	
			TWeakPtr<SMyBlueprint> MyBlueprint;
	};
}





