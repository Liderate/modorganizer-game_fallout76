#include "fallout76savegame.h"

#include "gamefallout76.h"

Fallout76SaveGame::Fallout76SaveGame(QString const& fileName, GameFallout76 const* game)
    : GamebryoSaveGame(fileName, game, true)
{
  FileWrapper file(fileName, "FO76_SAVEGAME");

  FILETIME ftime;
  fetchInformationFields(file, m_PCName, m_PCLevel, m_PCLocation, m_SaveNumber, ftime);

  // A file time is a 64-bit value that represents the number of 100-nanosecond
  // intervals that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal
  // Time (UTC). So we need to convert that to something useful
  SYSTEMTIME ctime;
  ::FileTimeToSystemTime(&ftime, &ctime);

  setCreationTime(ctime);
}

void Fallout76SaveGame::fetchInformationFields(FileWrapper& file, QString playerName,
                                               unsigned short playerLevel,
                                               QString playerLocation,
                                               unsigned long saveNumber,
                                               FILETIME& creationTime) const
{

  file.skip<unsigned long>();  // header size
  file.skip<uint32_t>();       // header version
  file.read(saveNumber);

  file.read(playerName);

  unsigned long temp;
  file.read(temp);
  playerLevel = static_cast<unsigned short>(temp);
  file.read(playerLocation);

  QString ignore;
  file.read(ignore);  // playtime as ascii hh.mm.ss
  file.read(ignore);  // race name (i.e. BretonRace)

  file.skip<unsigned short>();  // Player gender (0 = male)
  file.skip<float>(2);          // experience gathered, experience required

  FILETIME ftime;
  file.read(ftime);
}

std::unique_ptr<GamebryoSaveGame::DataFields> Fallout76SaveGame::fetchDataFields() const
{

  FileWrapper file(getFilepath(), "TESV_SAVEGAME");  // 10bytes
  {

    FILETIME ftime;
    fetchInformationFields(file, m_PCName, m_PCLevel, m_PCLocation, m_SaveNumber,
                           ftime);
  }

  std::unique_ptr<DataFields> fields = std::make_unique<DataFields>();

  file.readImage(384, true);

  uint8_t saveGameVersion = file.readChar();
  QString ignore;
  file.read(ignore);      // game version
  file.skip<uint32_t>();  // plugin info size

  file.readPlugins();
  if (saveGameVersion >= 68) {
    file.readLightPlugins();
  }
}
