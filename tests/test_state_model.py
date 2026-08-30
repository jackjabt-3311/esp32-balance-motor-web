import unittest


UINT32_MASK = 0xFFFFFFFF


class SafetyGateReference:
    """Reference state model for the embedded SafetyGate contract."""

    def __init__(self):
        self.helmet_worn = False
        self.helmet_seen = False
        self.last_helmet_ms = 0
        self.people = 0
        self.one_person_timing = False
        self.one_person_since_ms = 0

    @staticmethod
    def elapsed(now_ms, then_ms):
        return (now_ms - then_ms) & UINT32_MASK

    def note_helmet(self, worn, now_ms):
        self.helmet_seen = True
        self.helmet_worn = worn
        self.last_helmet_ms = now_ms

    def update_people(self, people, now_ms):
        self.people = people
        if people == 1:
            if not self.one_person_timing:
                self.one_person_timing = True
                self.one_person_since_ms = now_ms
        else:
            self.one_person_timing = False

    def evaluate(self, now_ms, encoder_calibrated, motor_fault):
        helmet_fresh = self.helmet_seen and self.elapsed(now_ms, self.last_helmet_ms) <= 500
        one_person_stable = (
            self.people == 1
            and self.one_person_timing
            and self.elapsed(now_ms, self.one_person_since_ms) >= 1000
        )

        if not helmet_fresh:
            reason = "helmet_signal_lost"
        elif not self.helmet_worn:
            reason = "helmet_not_worn"
        elif self.people != 1:
            reason = "people_not_one"
        elif not one_person_stable:
            reason = "people_not_stable"
        elif not encoder_calibrated:
            reason = "encoder_not_calibrated"
        elif motor_fault:
            reason = "motor_fault"
        else:
            reason = "ready"

        return {
            "helmetWorn": self.helmet_worn,
            "helmetFresh": helmet_fresh,
            "onePersonStable": one_person_stable,
            "encoderCalibrated": encoder_calibrated,
            "motorFault": motor_fault,
            "ready": reason == "ready",
            "lockReason": reason,
        }


class StateModelSmokeTests(unittest.TestCase):
    def test_module_loads_for_future_state_model_tests(self):
        self.assertTrue(True)


class EncoderReference:
    """Reference model for encoder sampling and one-revolution calibration."""

    RPM_SAMPLE_MS = 100
    MIN_PPR = 1
    MAX_PPR = 100000

    def __init__(self, ppr=0):
        self.ppr = ppr
        self.calibrating = False
        self.calibration_edges = 0
        self.last_sample_ms = 0
        self.last_pulse_count = 0
        self.current_rpm = 0.0
        self.pulse_count = 0
        self.finish_time_ms = 0
        self.persistence_succeeds = True

    def tick(self, now_ms, pulse_count):
        elapsed_ms = (now_ms - self.last_sample_ms) & UINT32_MASK
        if elapsed_ms == 0 or self.ppr == 0:
            self.current_rpm = 0.0
            return self.current_rpm
        if elapsed_ms < self.RPM_SAMPLE_MS:
            return self.current_rpm

        pulse_delta = pulse_count - self.last_pulse_count
        self.last_sample_ms = now_ms
        self.last_pulse_count = pulse_count
        self.current_rpm = abs(pulse_delta) * 60000.0 / (elapsed_ms * self.ppr)
        return self.current_rpm

    def start_calibration(self):
        if self.calibrating:
            return False, "already_calibrating"
        self.calibrating = True
        self.calibration_edges = 0
        return True, "calibration_started"

    def add_calibration_edge(self):
        if self.calibrating:
            self.calibration_edges += 1

    def finish_calibration(self):
        if not self.calibrating:
            return False, "not_calibrating"
        self.calibrating = False
        self.last_sample_ms = self.finish_time_ms
        self.last_pulse_count = self.pulse_count
        self.current_rpm = 0.0
        if not self.MIN_PPR <= self.calibration_edges <= self.MAX_PPR:
            return False, "invalid_pulse_count"
        if not self.persistence_succeeds:
            return False, "persistence_failed"
        self.ppr = self.calibration_edges
        return True, "calibration_saved"


class EncoderReferenceTests(unittest.TestCase):
    def test_100_edges_over_1000ms_at_100_ppr_is_60_rpm(self):
        encoder = EncoderReference(ppr=100)
        self.assertEqual(encoder.tick(1000, 100), 60.0)

    def test_zero_elapsed_or_zero_ppr_returns_zero_rpm(self):
        uncalibrated = EncoderReference(ppr=0)
        uncalibrated.current_rpm = 12.0
        self.assertEqual(uncalibrated.tick(99, 20), 0.0)
        encoder = EncoderReference(ppr=100)
        encoder.last_sample_ms = 100
        encoder.last_pulse_count = 20
        encoder.current_rpm = 12.0
        self.assertEqual(encoder.tick(100, 40), 0.0)

    def test_negative_edge_delta_uses_absolute_value(self):
        encoder = EncoderReference(ppr=100)
        encoder.last_pulse_count = 100
        self.assertEqual(encoder.tick(1000, 0), 60.0)

    def test_rpm_updates_no_faster_than_100ms(self):
        encoder = EncoderReference(ppr=100)
        self.assertEqual(encoder.tick(99, 100), 0.0)
        self.assertEqual(encoder.tick(100, 100), 600.0)

    def test_calibration_rejects_zero_and_100001_edges(self):
        for count in (0, 100001):
            encoder = EncoderReference(ppr=120)
            encoder.start_calibration()
            encoder.calibration_edges = count
            self.assertEqual(encoder.finish_calibration(), (False, "invalid_pulse_count"))

    def test_calibration_accepts_one_and_100000_edges(self):
        for count in (1, 100000):
            encoder = EncoderReference()
            encoder.start_calibration()
            encoder.calibration_edges = count
            self.assertEqual(encoder.finish_calibration(), (True, "calibration_saved"))
            self.assertEqual(encoder.ppr, count)

    def test_failed_calibration_preserves_existing_valid_ppr(self):
        encoder = EncoderReference(ppr=120)
        encoder.start_calibration()
        self.assertEqual(encoder.finish_calibration(), (False, "invalid_pulse_count"))
        self.assertEqual(encoder.ppr, 120)

    def test_successful_calibration_rebases_first_operational_rpm_sample(self):
        encoder = EncoderReference(ppr=120)
        encoder.last_sample_ms = 1000
        encoder.last_pulse_count = 50
        encoder.current_rpm = 42.0
        encoder.finish_time_ms = 5000
        encoder.pulse_count = 900
        encoder.start_calibration()
        encoder.calibration_edges = 100

        self.assertEqual(encoder.finish_calibration(), (True, "calibration_saved"))
        self.assertEqual((encoder.last_sample_ms, encoder.last_pulse_count, encoder.current_rpm), (5000, 900, 0.0))
        self.assertEqual(encoder.tick(5100, 920), 120.0)

    def test_persistence_failure_preserves_existing_valid_ppr(self):
        encoder = EncoderReference(ppr=120)
        encoder.persistence_succeeds = False
        encoder.start_calibration()
        encoder.calibration_edges = 100

        self.assertEqual(encoder.finish_calibration(), (False, "persistence_failed"))
        self.assertEqual(encoder.ppr, 120)

    def test_invalid_calibration_rebases_rpm_without_changing_existing_ppr(self):
        encoder = EncoderReference(ppr=120)
        encoder.last_sample_ms = 1000
        encoder.last_pulse_count = 50
        encoder.current_rpm = 42.0
        encoder.finish_time_ms = 5000
        encoder.pulse_count = 900
        encoder.start_calibration()

        self.assertEqual(encoder.finish_calibration(), (False, "invalid_pulse_count"))
        self.assertEqual(encoder.ppr, 120)
        self.assertEqual((encoder.last_sample_ms, encoder.last_pulse_count, encoder.current_rpm), (5000, 900, 0.0))
        self.assertEqual(encoder.tick(5100, 920), 100.0)

    def test_persistence_failure_rebases_rpm_without_changing_existing_ppr(self):
        encoder = EncoderReference(ppr=120)
        encoder.persistence_succeeds = False
        encoder.last_sample_ms = 1000
        encoder.last_pulse_count = 50
        encoder.current_rpm = 42.0
        encoder.finish_time_ms = 5000
        encoder.pulse_count = 900
        encoder.start_calibration()
        encoder.calibration_edges = 100

        self.assertEqual(encoder.finish_calibration(), (False, "persistence_failed"))
        self.assertEqual(encoder.ppr, 120)
        self.assertEqual((encoder.last_sample_ms, encoder.last_pulse_count, encoder.current_rpm), (5000, 900, 0.0))
        self.assertEqual(encoder.tick(5100, 920), 100.0)


class SafetyGateReferenceTests(unittest.TestCase):
    def test_no_helmet_packet_locks_for_lost_signal(self):
        gate = SafetyGateReference()
        gate.update_people(1, 0)

        snapshot = gate.evaluate(1000, True, False)

        self.assertFalse(snapshot["ready"])
        self.assertEqual(snapshot["lockReason"], "helmet_signal_lost")

    def test_worn_helmet_and_one_person_at_999ms_are_not_stable(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 700)
        gate.update_people(1, 0)

        snapshot = gate.evaluate(999, True, False)

        self.assertFalse(snapshot["onePersonStable"])
        self.assertEqual(snapshot["lockReason"], "people_not_stable")

    def test_refreshed_worn_helmet_and_one_person_at_1000ms_are_ready(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 500)
        gate.update_people(1, 0)

        snapshot = gate.evaluate(1000, True, False)

        self.assertTrue(snapshot["ready"])
        self.assertEqual(snapshot["lockReason"], "ready")

    def test_helmet_packet_older_than_timeout_locks_for_lost_signal(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 0)
        gate.update_people(1, 0)

        snapshot = gate.evaluate(501, True, False)

        self.assertFalse(snapshot["helmetFresh"])
        self.assertEqual(snapshot["lockReason"], "helmet_signal_lost")

    def test_explicit_unworn_helmet_has_priority_after_fresh_packet(self):
        gate = SafetyGateReference()
        gate.note_helmet(False, 0)
        gate.update_people(1, 0)

        snapshot = gate.evaluate(500, True, False)

        self.assertEqual(snapshot["lockReason"], "helmet_not_worn")

    def test_non_one_people_count_revokes_qualification_immediately(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 1000)
        gate.update_people(1, 0)
        self.assertTrue(gate.evaluate(1000, True, False)["onePersonStable"])

        gate.update_people(2, 1000)
        snapshot = gate.evaluate(1000, True, False)

        self.assertFalse(snapshot["onePersonStable"])
        self.assertEqual(snapshot["lockReason"], "people_not_one")

    def test_repeated_one_person_updates_do_not_postpone_qualification(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 700)
        gate.update_people(1, 0)
        gate.update_people(1, 500)
        gate.update_people(1, 999)

        snapshot = gate.evaluate(1000, True, False)

        self.assertTrue(snapshot["onePersonStable"])
        self.assertTrue(snapshot["ready"])

    def test_encoder_precedes_motor_fault_in_lock_reason_priority(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 1000)
        gate.update_people(1, 0)

        self.assertEqual(gate.evaluate(1000, False, True)["lockReason"], "encoder_not_calibrated")
        self.assertEqual(gate.evaluate(1000, True, True)["lockReason"], "motor_fault")

    def test_elapsed_time_is_rollover_safe_for_helmet_and_people(self):
        gate = SafetyGateReference()
        gate.note_helmet(True, 0xFFFFFF00)
        gate.update_people(1, 0xFFFFFC00)

        self.assertTrue(gate.evaluate(0x000000F4, True, False)["ready"])
        self.assertEqual(gate.evaluate(0x000000F5, True, False)["lockReason"], "helmet_signal_lost")


if __name__ == "__main__":
    unittest.main()
