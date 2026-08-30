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
