package com.smartroommonitor.domain.usecase

/** Batas kesegaran data (SRS v1.0 §7.3, FR-B02). */
const val STALE_AFTER_MS = 15_000L

enum class Freshness { Fresh, Stale, NoData }

fun freshnessOf(timestampMs: Long, nowMs: Long): Freshness = when {
    timestampMs <= 0L -> Freshness.NoData
    nowMs - timestampMs > STALE_AFTER_MS -> Freshness.Stale
    else -> Freshness.Fresh
}
