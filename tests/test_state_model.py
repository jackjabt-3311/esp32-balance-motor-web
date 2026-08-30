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


class MotorReference:
    """Executable reference for the motor command, ramp, PID, and fault contract."""

    PID_PERIOD_MS = 50
    RAMP_DOWN_MS = 2000
    STALL_TIMEOUT_MS = 1000
    KP = 0.8
    KI = 0.15

    def __init__(self):
        self.state = "ready"
        self.target = 0
        self.duty = 0.0
        self.integral = 0.0
        self.last_pid_ms = 0
        self.pid_started = False
        self.ramp_started_ms = 0
        self.ramp_start_duty = 0.0
        self.stall_started_ms = 0
        self.stall_timing = False
        self.fault_reason = ""

    @staticmethod
    def elapsed(now_ms, then_ms):
        return (now_ms - then_ms) & UINT32_MASK

    def request_on(self, safety_ready):
        if self.state == "ramping_down":
            return False, "ramping"
        if self.state == "estop":
            return False, "estop_latched"
        if self.state == "fault":
            return False, "fault_latched"
        if self.state != "ready":
            return False, "invalid_state"
        if not safety_ready:
            self.state = "locked"
            return False, "safety_locked"
        self.target = 0
        self.state = "armed"
        return True, "armed"

    def set_target(self, rpm):
        if rpm > 300:
            return False, "rpm_out_of_range"
        if self.state == "ramping_down":
            return False, "ramping"
        if self.state == "estop":
            return False, "estop_latched"
        if self.state == "fault":
            return False, "fault_latched"
        if self.state not in ("armed", "running"):
            return False, "invalid_state"
        self.target = rpm
        self.state = "running" if rpm else "armed"
        if rpm == 0:
            self.duty = 0.0
            self._reset_pid()
        return True, "running" if rpm else "armed"

    def request_off(self, now_ms):
        if self.state == "ramping_down":
            return False, "ramping"
        if self.state == "estop":
            return False, "estop_latched"
        if self.state == "fault":
            return False, "fault_latched"
        if self.state not in ("armed", "running"):
            return False, "invalid_state"
        self._start_ramp(now_ms)
        return True, "ramping"

    def request_estop(self):
        self._stop()
        self.state = "estop"
        self.fault_reason = "estop"
        return True, "estop"

    def request_reset(self, safety_ready):
        if self.state == "ramping_down":
            return False, "ramping"
        if self.duty != 0:
            return False, "output_active"
        self._stop()
        self.fault_reason = ""
        self.state = "ready" if safety_ready else "locked"
        return True, "ready" if safety_ready else "safety_locked"

    def _reset_pid(self):
        self.integral = 0.0
        self.last_pid_ms = 0
        self.pid_started = False

    def _stop(self):
        self.duty = 0.0
        self.target = 0
        self.stall_timing = False
        self._reset_pid()

    def _start_ramp(self, now_ms):
        self.ramp_started_ms = now_ms
        self.ramp_start_duty = self.duty
        self.target = 0
        self.state = "ramping_down"
        self.stall_timing = False
        self._reset_pid()

    def tick(self, now_ms, safety_ready, actual_rpm):
        if self.state in ("estop", "fault"):
            return
        if self.state == "ramping_down":
            elapsed = self.elapsed(now_ms, self.ramp_started_ms)
            fraction = min(1.0, elapsed / self.RAMP_DOWN_MS)
            self.duty = self.ramp_start_duty * (1.0 - fraction)
            if elapsed >= self.RAMP_DOWN_MS:
                self._stop()
                self.state = "ready" if safety_ready else "locked"
            return
        if self.state in ("armed", "running") and not safety_ready:
            self._start_ramp(now_ms)
            return
        if self.state in ("ready", "locked"):
            self.state = "ready" if safety_ready else "locked"
            return
        if self.state != "running":
            return

        if not self.pid_started:
            self.pid_started = True
            self.last_pid_ms = now_ms
        else:
            elapsed = self.elapsed(now_ms, self.last_pid_ms)
            if elapsed >= self.PID_PERIOD_MS:
                error = self.target - actual_rpm
                candidate_integral = self.integral + error * (elapsed / 1000.0)
                candidate_integral = max(0.0, min(100.0, candidate_integral))
                candidate_output = self.KP * error + self.KI * candidate_integral
                saturating_high = candidate_output > 100.0 and error > 0.0
                saturating_low = candidate_output < 0.0 and error < 0.0
                if not saturating_high and not saturating_low:
                    self.integral = candidate_integral
                self.duty = max(0.0, min(100.0, self.KP * error + self.KI * self.integral))
                self.last_pid_ms = now_ms

        stalled = self.target >= 20 and self.duty >= 25 and actual_rpm < 5
        if not stalled:
            self.stall_timing = False
        elif not self.stall_timing:
            self.stall_timing = True
            self.stall_started_ms = now_ms
        elif self.elapsed(now_ms, self.stall_started_ms) >= self.STALL_TIMEOUT_MS:
            self._stop()
            self.state = "fault"
            self.fault_reason = "encoder_stall"


class MotorReferenceTests(unittest.TestCase):
    def test_on_arms_at_zero_then_positive_slider_runs(self):
        motor = MotorReference()
        self.assertEqual(motor.request_on(True), (True, "armed"))
        self.assertEqual((motor.state, motor.target, motor.duty), ("armed", 0, 0.0))
        self.assertEqual(motor.set_target(120), (True, "running"))
        self.assertEqual((motor.state, motor.target), ("running", 120))

    def test_target_accepts_bounds_and_rejects_301(self):
        motor = MotorReference()
        motor.request_on(True)
        self.assertTrue(motor.set_target(0)[0])
        self.assertTrue(motor.set_target(300)[0])
        self.assertEqual(motor.set_target(301), (False, "rpm_out_of_range"))

    def test_off_ramps_from_current_duty_over_two_seconds(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(100)
        motor.duty = 80.0
        self.assertEqual(motor.request_off(0xFFFFFF00), (True, "ramping"))
        motor.tick(0x000002E8, True, 0)  # 1000 ms after rollover
        self.assertAlmostEqual(motor.duty, 40.0)
        motor.tick(0x000006D0, True, 0)
        self.assertEqual((motor.state, motor.duty, motor.target), ("ready", 0.0, 0))

    def test_safety_loss_ramps_and_recovery_does_not_restart(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(150)
        motor.duty = 60.0
        motor.tick(100, False, 100)
        self.assertEqual(motor.state, "ramping_down")
        motor.tick(2100, True, 0)
        self.assertEqual((motor.state, motor.target, motor.duty), ("ready", 0, 0.0))
        motor.tick(2200, True, 0)
        self.assertEqual((motor.state, motor.target), ("ready", 0))

    def test_on_and_slider_requests_are_rejected_during_ramp(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(100)
        motor.request_off(0)
        self.assertEqual(motor.request_on(True), (False, "ramping"))
        self.assertEqual(motor.set_target(50), (False, "ramping"))
        self.assertEqual(motor.request_off(100), (False, "ramping"))

    def test_estop_is_immediate_and_latched(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(100)
        motor.duty = 70.0
        self.assertEqual(motor.request_estop(), (True, "estop"))
        self.assertEqual((motor.state, motor.duty, motor.target), ("estop", 0.0, 0))
        motor.tick(5000, True, 100)
        self.assertEqual(motor.state, "estop")
        self.assertEqual(motor.request_on(True), (False, "estop_latched"))
        self.assertEqual(motor.set_target(50), (False, "estop_latched"))

    def test_encoder_stall_is_immediate_after_one_second_and_latched(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(100)
        motor.duty = 50.0
        motor.tick(0xFFFFFF00, True, 0)
        motor.tick(0x000002E7, True, 0)
        self.assertEqual(motor.state, "running")
        motor.tick(0x000002E8, True, 0)
        self.assertEqual((motor.state, motor.duty, motor.target), ("fault", 0.0, 0))
        self.assertEqual(motor.fault_reason, "encoder_stall")
        motor.tick(5000, True, 100)
        self.assertEqual(motor.state, "fault")
        self.assertEqual(motor.request_on(True), (False, "fault_latched"))
        self.assertEqual(motor.set_target(50), (False, "fault_latched"))

    def test_reset_requires_zero_output_and_uses_current_safety(self):
        motor = MotorReference()
        motor.duty = 1.0
        motor.state = "fault"
        self.assertEqual(motor.request_reset(True), (False, "output_active"))
        motor.duty = 0.0
        self.assertEqual(motor.request_reset(False), (True, "safety_locked"))
        self.assertEqual(motor.state, "locked")
        self.assertEqual(motor.request_reset(True), (True, "ready"))
        self.assertEqual(motor.state, "ready")

    def test_zero_duty_ramp_rejects_reset_until_two_seconds_complete(self):
        motor = MotorReference()
        motor.request_on(True)
        self.assertEqual(motor.duty, 0.0)
        self.assertEqual(motor.request_off(100), (True, "ramping"))
        self.assertEqual(motor.request_reset(True), (False, "ramping"))
        motor.tick(2099, True, 0)
        self.assertEqual(motor.request_reset(True), (False, "ramping"))
        motor.tick(2100, True, 0)
        self.assertEqual(motor.request_reset(True), (True, "ready"))

    def test_pid_cadence_clamps_and_stopped_reset(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(300)
        motor.tick(100, True, 0)
        self.assertEqual(motor.duty, 0.0)
        motor.tick(149, True, 0)
        self.assertEqual(motor.duty, 0.0)
        motor.tick(150, True, 0)
        self.assertEqual(motor.duty, 100.0)
        motor.tick(200, True, 0)
        motor.tick(250, True, 0)
        self.assertEqual(motor.integral, 0.0)
        self.assertEqual(motor.duty, 100.0)
        motor.tick(300, True, 400)
        self.assertEqual(motor.integral, 0.0)
        self.assertEqual(motor.duty, 0.0)
        motor.set_target(0)
        self.assertEqual((motor.duty, motor.integral, motor.pid_started), (0.0, 0.0, False))

    def test_near_setpoint_positive_error_never_integrates_above_duty_range(self):
        motor = MotorReference()
        motor.request_on(True)
        motor.set_target(100)
        motor.tick(0, True, 99)
        for now_ms in range(50, 150001, 50):
            motor.tick(now_ms, True, 99)
        self.assertEqual(motor.integral, 100.0)
        self.assertLessEqual(motor.duty, 100.0)


if __name__ == "__main__":
    unittest.main()
