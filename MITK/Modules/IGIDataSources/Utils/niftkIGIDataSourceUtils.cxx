/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
#include "niftkIGIDataSourceUtils.h"
#include <niftkFileHelper.h>

namespace niftk
{

//-----------------------------------------------------------------------------
QString GetPreferredSlash()
{
  return QString(QDir::separator());
}


//-----------------------------------------------------------------------------
void ProbeTimeStampFiles(QDir path,
                         const QString& suffix,
                         std::set<niftk::IGIDataSourceI::IGITimeType>& timeStamps
                         )
{
  QHash<niftk::IGIDataSourceI::IGITimeType, QString> timeStampToFileName; // unused.
  niftk::ProbeTimeStampFiles(path, suffix, timeStamps, timeStampToFileName);
}


//-----------------------------------------------------------------------------
void ProbeTimeStampFiles(QDir path,
                         const QString& suffix,
                         std::set<niftk::IGIDataSourceI::IGITimeType>& timeStamps,
                         QHash<niftk::IGIDataSourceI::IGITimeType, QString>& timeStampToFileName)
{
  assert(std::numeric_limits<qulonglong>::max() >= std::numeric_limits<niftk::IGIDataSourceI::IGITimeType>::max());

  if (!suffix.isEmpty())
  {
    QStringList filters;
    filters << QString("*" + suffix);
    path.setNameFilters(filters);
  }
  path.setFilter(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);

  QStringList files = path.entryList();
  if (!files.empty())
  {
    for (QString file: files)
    {
      if (file.length() >= 19 + suffix.length())
      {
        QString  middle = file.mid(0, 19);

        bool  ok = false;
        qulonglong value = middle.toULongLong(&ok);
        if (ok)
        {
          timeStamps.insert(value);
          timeStampToFileName.insert(value, path.absolutePath() + QDir::separator() + file);
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
void GetPlaybackIndex(const QString& directory,
                      const QString& fileExtension,
                      QMap<QString, std::set<niftk::IGIDataSourceI::IGITimeType> >& bufferToTimeStamp,
                      QMap<QString, QHash<
                        niftk::IGIDataSourceI::IGITimeType, QStringList> >& bufferToTimeStampToFileNames
                      )
{
  bufferToTimeStamp.clear();
  bufferToTimeStampToFileNames.clear();

  QDir recordingDir(directory);
  if (recordingDir.exists())
  {
    // So here, inside the recordingDir will be sub-folders.
    // These subFolders should correspond to each buffer.
    // The buffer corresponds to a device. e.g. Aurora, Polaris, Ultrasonix.

    recordingDir.setFilter(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
    QStringList bufferLevelNames = recordingDir.entryList();

    if (!bufferLevelNames.isEmpty())
    {
      for (QString bufferLevelName: bufferLevelNames)
      {
        QDir  bufferLevelDir(recordingDir.path() + QDir::separator() + bufferLevelName);
        assert(bufferLevelDir.exists());

        std::set<niftk::IGIDataSourceI::IGITimeType> timeStamps;
        QHash<niftk::IGIDataSourceI::IGITimeType, QString> timeStampsToFileName;
        niftk::ProbeTimeStampFiles(bufferLevelDir, fileExtension, timeStamps, timeStampsToFileName);

        // If the bufferLevel folder contains nothing that looks like a time-stamped file,
        // then we start to hunt for sub-directories below that.
        if (timeStamps.empty())
        {
          QMap<QString, std::set<niftk::IGIDataSourceI::IGITimeType> > subBufferToTimeStamp;
          QMap<QString, QHash<niftk::IGIDataSourceI::IGITimeType, QStringList> > subBufferToTimeStampToFileNames;

          GetPlaybackIndex(bufferLevelDir.absolutePath(),
                           fileExtension,
                           subBufferToTimeStamp,
                           subBufferToTimeStampToFileNames);

          if (!subBufferToTimeStamp.isEmpty())
          {
            std::set<niftk::IGIDataSourceI::IGITimeType> combinedSet;
            QHash<niftk::IGIDataSourceI::IGITimeType, QStringList> combinedHash;

            // Iterate through all timestamps, and combine.
            QMap<QString, std::set<niftk::IGIDataSourceI::IGITimeType> >::iterator iter;
            for (iter = subBufferToTimeStamp.begin(); iter != subBufferToTimeStamp.end(); ++iter)
            {
              combinedSet.insert(iter.value().begin(), iter.value().end());
            }

            // Iterate through all timestamps, combining filenames.
            std::set<niftk::IGIDataSourceI::IGITimeType>::iterator iter2;
            for (iter2 = combinedSet.begin(); iter2 != combinedSet.end(); ++iter2)
            {
              QStringList tmp;

              QMap<QString, QHash<niftk::IGIDataSourceI::IGITimeType, QStringList> >::iterator iter3;
              for (iter3 = subBufferToTimeStampToFileNames.begin();
                   iter3 != subBufferToTimeStampToFileNames.end();
                   ++iter3)
              {
                if (iter3.value().contains(*iter2))
                {
                  if (iter3.value().value(*iter2).size() > 0)
                  {
                    tmp << iter3.value().value(*iter2);
                  }
                }
              }
              if (tmp.size() > 0)
              {
                combinedHash.insert(*iter2, tmp);
              }
            }
            bufferToTimeStampToFileNames.insert(bufferLevelName, combinedHash);
            bufferToTimeStamp.insert(bufferLevelName, combinedSet);
          }
        }
        else
        {
          QHash<niftk::IGIDataSourceI::IGITimeType, QStringList> timeStampsToFileNames;

          // Convert single file name to stringlist
          QHash<niftk::IGIDataSourceI::IGITimeType, QString>::iterator iter;
          for (iter = timeStampsToFileName.begin(); iter != timeStampsToFileName.end(); ++iter)
          {
            QStringList list;
            list << iter.value();
            timeStampsToFileNames.insert(iter.key(), list);
          }
          bufferToTimeStampToFileNames.insert(bufferLevelName, timeStampsToFileNames);
          bufferToTimeStamp.insert(bufferLevelName, timeStamps);
        }
      }
    }
    else
    {
      MITK_WARN << "There are no sub-folders in " << recordingDir.absolutePath().toStdString()
                << ", so can't playback data!";
    }
  }
  else
  {
    mitkThrow() << "Recording directory, " << recordingDir.absolutePath().toStdString() << ", does not exist!";
  }
  if (bufferToTimeStamp.isEmpty())
  {
    mitkThrow() << "No data extracted from directory " << recordingDir.absolutePath().toStdString();
  }
}


//-----------------------------------------------------------------------------
bool ProbeRecordedData(const QString& path,
                       const QString& fileExtension,
                       niftk::IGIDataSourceI::IGITimeType* firstTimeStampInStore,
                       niftk::IGIDataSourceI::IGITimeType* lastTimeStampInStore)
{

  niftk::IGIDataSourceI::IGITimeType  firstTimeStampFound
    = std::numeric_limits<niftk::IGIDataSourceI::IGITimeType>::max();

  niftk::IGIDataSourceI::IGITimeType  lastTimeStampFound
    = std::numeric_limits<niftk::IGIDataSourceI::IGITimeType>::min();

  // Note, that each tool may have different min and max, so we want the
  // most minimum and most maximum of all the sub directories.
  QMap<QString, std::set<niftk::IGIDataSourceI::IGITimeType> > bufferToTimeStamp;
  QMap<QString, QHash<niftk::IGIDataSourceI::IGITimeType, QStringList> > bufferToTimeStampToFileNames;

  niftk::GetPlaybackIndex(path, fileExtension, bufferToTimeStamp, bufferToTimeStampToFileNames);
  if (bufferToTimeStamp.isEmpty())
  {
    return false;
  }

  QMap<QString, std::set<niftk::IGIDataSourceI::IGITimeType> >::iterator iter;
  for (iter = bufferToTimeStamp.begin(); iter != bufferToTimeStamp.end(); ++iter)
  {
    if (!iter.value().empty())
    {
      niftk::IGIDataSourceI::IGITimeType first = *((*iter).begin());
      if (first < firstTimeStampFound)
      {
        firstTimeStampFound = first;
      }

      niftk::IGIDataSourceI::IGITimeType last = *(--((*iter).end()));
      if (last > lastTimeStampFound)
      {
        lastTimeStampFound = last;
      }
    }
  }
  if (firstTimeStampInStore)
  {
    *firstTimeStampInStore = firstTimeStampFound;
  }
  if (lastTimeStampInStore)
  {
    *lastTimeStampInStore = lastTimeStampFound;
  }
  return firstTimeStampFound != std::numeric_limits<niftk::IGIDataSourceI::IGITimeType>::max();
}

} // end namespace
