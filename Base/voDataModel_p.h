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

#ifndef __voDataModel_p_h
#define __voDataModel_p_h

// Qt includes
#include <QObject>
#include <QModelIndex>

class QItemSelectionModel;
class voAnalysis;
class voDataModel;
class voDataModelItem;

class voDataModelPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(voDataModel);
protected:
  voDataModel* const q_ptr;
public:
  voDataModelPrivate(voDataModel& object);
  virtual ~voDataModelPrivate();

public slots:

  void onCurrentRowChanged(const QModelIndex & current, const QModelIndex & previous);

public:

  QItemSelectionModel*          SelectionModel;
  QList<voDataModelItem*>       SelectedInputDataObjects;
  voDataModelItem*              SelectedDataObject;
  QHash<QString, unsigned int>  NameCountMap;
  voAnalysis*                   ActiveAnalysis;
  QList<voDataModelItem*>       Analyses;
};

#endif
