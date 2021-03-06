/*=========================================================================

  Program: Visomics

  Copyright (c) Kitware, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QDebug>
#include <QLayout>

// Visomics includes
#include "voDataObject.h"
#include "voTreeHeatmapView.h"
#include "vtkExtendedTable.h"

// VTK includes
#include <QVTKWidget.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkGraph.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkContextView.h>
#include <vtkTree.h>
#include <vtkViewTheme.h>
#include <vtkTreeHeatmapItem.h>
#include <vtkContextItem.h>
#include <vtkContextScene.h>
#include <vtkContextTransform.h>
#include <vtkNew.h>

// --------------------------------------------------------------------------
class voTreeHeatmapViewPrivate
{
public:
  voTreeHeatmapViewPrivate();
  vtkSmartPointer<vtkContextView>      ContextView;
  vtkSmartPointer<vtkTreeHeatmapItem>  TreeItem;
  vtkSmartPointer<vtkContextTransform> TransformItem;
  QVTKWidget*                          Widget;
};

// --------------------------------------------------------------------------
// voTreeHeatmapViewPrivate methods

// --------------------------------------------------------------------------
voTreeHeatmapViewPrivate::voTreeHeatmapViewPrivate()
{
  this->Widget = 0;
}

// --------------------------------------------------------------------------
// voTreeHeatmapView methods

// --------------------------------------------------------------------------
voTreeHeatmapView::voTreeHeatmapView(QWidget * newParent):
    Superclass(newParent), d_ptr(new voTreeHeatmapViewPrivate)
{
}

// --------------------------------------------------------------------------
voTreeHeatmapView::~voTreeHeatmapView()
{
}

// --------------------------------------------------------------------------
vtkTreeHeatmapItem * voTreeHeatmapView::getTreeHeatmapItem() const
{
  Q_D(const voTreeHeatmapView);
  return d->TreeItem;
}
// --------------------------------------------------------------------------
void voTreeHeatmapView::setupUi(QLayout *layout)
{
  Q_D(voTreeHeatmapView);

  d->ContextView = vtkSmartPointer<vtkContextView>::New();
  d->Widget = new QVTKWidget();
  d->ContextView->SetInteractor(d->Widget->GetInteractor());
  d->Widget->SetRenderWindow(d->ContextView->GetRenderWindow());
  d->ContextView->GetRenderer()->SetBackground(1.0, 1.0, 1.0);

  d->TreeItem = vtkSmartPointer<vtkTreeHeatmapItem>::New();
  d->TransformItem = vtkSmartPointer<vtkContextTransform>::New();
  d->TransformItem->AddItem(d->TreeItem);
  d->TransformItem->SetInteractive(true);
  d->ContextView->GetScene()->AddItem(d->TransformItem);

  layout->addWidget(d->Widget);
}

// --------------------------------------------------------------------------
void voTreeHeatmapView::setDataObjectListInternal(const QList<voDataObject*> dataObjects)
{
  Q_D(voTreeHeatmapView);
  if (dataObjects.size() == 2)
    {
    vtkTree * Tree = vtkTree::SafeDownCast(dataObjects[0]->dataAsVTKDataObject());
    if (!Tree)
      {
      qCritical() << "voTreeHeatmapView - Failed to setDataObject - vtkTree data is expected !";
      return;
      }

    vtkExtendedTable * Table = vtkExtendedTable::SafeDownCast(dataObjects[1]->dataAsVTKDataObject());
    if (!Table)
      {
      qCritical() << "voTreeHeatmapView - Failed to setDataObject - vtkExtendedTable data is expected !";
      return;
      }
    //If there is a real change in the tree (# of vertices), update the tree, otherwise, just leave it there,
    //so that expanding/collapsing interactions results could be kept on the view.
    if (d->TreeItem->GetTree()->GetNumberOfVertices() != Tree->GetNumberOfVertices())
      {
      // making a copy of the Tree so that the TreeHeatmap & Dendrogram no longer
      // share a single input data source
      vtkSmartPointer<vtkTree> treeCopy = vtkSmartPointer<vtkTree>::New();
      treeCopy->DeepCopy(Tree);
      d->TreeItem->SetTree(treeCopy);
      }
    if (d->TreeItem->GetTable() != Table->GetInputData())
      {
      d->TreeItem->SetTable(Table->GetInputData());
      }
    }
  else
    {
    // one of our dataObjects must have been deleted...
    // first check if we have ANY data left
    if (dataObjects.size() == 0)
      {
      return;
      }

    vtkTree * Tree = vtkTree::SafeDownCast(dataObjects[0]->dataAsVTKDataObject());
    if (Tree)
      {
      // do we have a tree?
      if (d->TreeItem->GetTree() != Tree)
        {
        // is it different from the one we're already drawing?
        d->TreeItem->SetTree(Tree);
        }
      d->TreeItem->SetTable(NULL);
      }
    else
      {
      // or do we have a table?
      vtkExtendedTable * Table = vtkExtendedTable::SafeDownCast(dataObjects[0]->dataAsVTKDataObject());
      if (!Table)
        {
        qCritical() << "voTreeHeatmapView - Failed to setDataObjectListInternal - Neither tree or table were found !";
        return;
        }
      if (d->TreeItem->GetTable() != Table->GetInputData())
        {
        // and is it different from the one we're already drawing?
        d->TreeItem->SetTable(Table->GetInputData());
        }
      d->TreeItem->SetTree(NULL);
      }
    }

  this->colorTreeForDifference();
  d->ContextView->GetRenderWindow()->SetMultiSamples(0);
  d->ContextView->Render();
}

// --------------------------------------------------------------------------
void voTreeHeatmapView::setDataObjectInternal(const voDataObject& dataObject)
{//only render the tree
  Q_D(voTreeHeatmapView);

  vtkTree * tree = vtkTree::SafeDownCast(dataObject.dataAsVTKDataObject());
  if (!tree)
    {
    qCritical() << "voTreeHeatmapView - Failed to setDataObject - vtkTree data is expected !";
    return;
    }

  if (d->TreeItem->GetTree() != tree)
    {
    d->TreeItem->SetTree(tree);
    }
  this->colorTreeForDifference();
  d->ContextView->GetRenderWindow()->SetMultiSamples(0);
  d->ContextView->Render();
}

// --------------------------------------------------------------------------
void voTreeHeatmapView::colorTreeForDifference()
{
  Q_D(voTreeHeatmapView);
  vtkDataArray *differenceArray =
    d->TreeItem->GetTree()->GetVertexData()->GetArray("differences");
  if (differenceArray)
    {
    d->TreeItem->SetTreeColorArray("differences");
    }
}
