package com.smartroommonitor.data.local

import androidx.room.Dao
import androidx.room.Database
import androidx.room.Entity
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.PrimaryKey
import androidx.room.Query
import androidx.room.RoomDatabase
import androidx.room.migration.Migration
import androidx.sqlite.db.SupportSQLiteDatabase
import com.smartroommonitor.domain.model.HistoryPoint
import com.smartroommonitor.domain.model.RoomCurrent
import kotlinx.coroutines.flow.Flow

@Entity(tableName = "current_snapshot")
data class CurrentSnapshotEntity(
    @PrimaryKey val id: Int = 1,
    val temperature: Double,
    val humidity: Double,
    val humidityLabel: String,
    val airDelta: Int,
    val airLabel: String,
    val airAqi: Int,
    val airTvoc: Int,
    val airEco2: Int,
    val airWarmingUp: Boolean,
    val lightLux: Double,
    val lightLabel: String,
    val presence: String,
    val presenceDetected: Boolean,
    val fanStatus: Boolean,
    val lampStatus: Boolean,
    val nightModeActive: Boolean,
    val timestampMs: Long,
)

/** Cache 24 jam grafik. Maksimal 144 baris; yang lebih lama dihapus. */
@Entity(tableName = "history_point")
data class HistoryPointEntity(
    @PrimaryKey val timestampMs: Long,
    val keyEpochSec: Long,
    val temperature: Double,
    val humidity: Double,
    val airDelta: Int,
    val airAqi: Int,
    val lightLux: Double,
)

@Dao
interface CurrentSnapshotDao {
    @Query("SELECT * FROM current_snapshot WHERE id = 1")
    fun observe(): Flow<CurrentSnapshotEntity?>

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun upsert(entity: CurrentSnapshotEntity)
}

@Dao
interface HistoryDao {
    @Query("SELECT * FROM history_point ORDER BY timestampMs ASC")
    fun observe(): Flow<List<HistoryPointEntity>>

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun upsertAll(entities: List<HistoryPointEntity>)

    @Query("DELETE FROM history_point WHERE timestampMs < :cutoffMs")
    suspend fun pruneBefore(cutoffMs: Long)
}

@Database(
    entities = [CurrentSnapshotEntity::class, HistoryPointEntity::class],
    version = 2,
    exportSchema = true,
)
abstract class AppDatabase : RoomDatabase() {
    abstract fun currentSnapshotDao(): CurrentSnapshotDao
    abstract fun historyDao(): HistoryDao
}

val MIGRATION_1_2 = object : Migration(1, 2) {
    override fun migrate(db: SupportSQLiteDatabase) {
        db.execSQL(
            "CREATE TABLE IF NOT EXISTS `history_point` (" +
                "`timestampMs` INTEGER NOT NULL, " +
                "`keyEpochSec` INTEGER NOT NULL, " +
                "`temperature` REAL NOT NULL, " +
                "`humidity` REAL NOT NULL, " +
                "`airDelta` INTEGER NOT NULL, " +
                "`airAqi` INTEGER NOT NULL, " +
                "`lightLux` REAL NOT NULL, " +
                "PRIMARY KEY(`timestampMs`))"
        )
    }
}

fun RoomCurrent.toEntity() = CurrentSnapshotEntity(
    temperature = temperature,
    humidity = humidity,
    humidityLabel = humidityLabel,
    airDelta = airDelta,
    airLabel = airLabel,
    airAqi = airAqi,
    airTvoc = airTvoc,
    airEco2 = airEco2,
    airWarmingUp = airWarmingUp,
    lightLux = lightLux,
    lightLabel = lightLabel,
    presence = presence,
    presenceDetected = presenceDetected,
    fanStatus = fanStatus,
    lampStatus = lampStatus,
    nightModeActive = nightModeActive,
    timestampMs = timestampMs,
)

fun CurrentSnapshotEntity.toModel() = RoomCurrent(
    temperature = temperature,
    humidity = humidity,
    humidityLabel = humidityLabel,
    airDelta = airDelta,
    airLabel = airLabel,
    airAqi = airAqi,
    airTvoc = airTvoc,
    airEco2 = airEco2,
    airWarmingUp = airWarmingUp,
    lightLux = lightLux,
    lightLabel = lightLabel,
    presence = presence,
    presenceDetected = presenceDetected,
    fanStatus = fanStatus,
    lampStatus = lampStatus,
    nightModeActive = nightModeActive,
    timestampMs = timestampMs,
)

fun HistoryPoint.toEntity() = HistoryPointEntity(
    timestampMs = timestampMs,
    keyEpochSec = keyEpochSec,
    temperature = temperature,
    humidity = humidity,
    airDelta = airDelta,
    airAqi = airAqi,
    lightLux = lightLux,
)

fun HistoryPointEntity.toModel() = HistoryPoint(
    keyEpochSec = keyEpochSec,
    temperature = temperature,
    humidity = humidity,
    airDelta = airDelta,
    airAqi = airAqi,
    lightLux = lightLux,
    timestampMs = timestampMs,
)
