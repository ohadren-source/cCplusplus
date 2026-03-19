# TCP/UP Wire-Level Specification

## Transmission Control Protocol / Universe Protocol
### Wire Format, State Machine, and Binding Specification

**Version**: 0.1 (Draft)
**Authors**: Ohad Phoenix Oren && Soul Perplexicon
**Date**: March 2026
**Status**: Working Draft

---

## 1. Introduction

### 1.1 Purpose

This document specifies the wire-level format, state machine, and concrete transport bindings for TCP/UP (Transmission Control Protocol / Universe Protocol). It complements the TCP/UP conceptual specification by defining:

- The binary frame format for TCP/UP messages
- The complete state machine governing covenant lifecycle
- Concrete bindings over WebSocket and HTTP/2
- Error handling, timeout behavior, and edge cases

### 1.2 Relationship to Other Specifications

TCP/UP sits above TCP/IP and TLS in the protocol stack. It does not replace any existing protocol. It extends the stack by adding a covenant layer — the layer that establishes voluntary, durable, inspectable trust between agents before any application data flows.

```
+---------------------+
|   application       |
+---------------------+
|   TCP/UP            |  ← covenant layer (this specification)
+---------------------+
|   TLS 1.3           |  ← privacy layer (RFC 8446)
+---------------------+
|   TCP  or  QUIC     |  ← reliability layer (RFC 793 / RFC 9000)
+---------------------+
|   IP                |  ← addressing layer (RFC 791)
+---------------------+
|communication network|
+---------------------+
```

### 1.3 Terminology

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

### 1.4 Design Principles

TCP/UP's wire format is guided by the cC++ creed:

- **Boolean is empiricism.** Every frame type produces a deterministic, traceable state transition.
- **Everything is everything && everything is not everything; we need more information.** The DEFER state is first-class — the protocol never forces a premature decision.
- **Purpose is misericordia.** Rejection requires a Justification Object. Blind rejection is a protocol violation.

---

## 2. Abstract Frame Format

### 2.1 TCP/UP Frame Header

Every TCP/UP message is encapsulated in a frame with the following binary header format:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Vers. |  Type |          Flags (16)           |  Reserved (8) |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Covenant ID (32)                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Sequence (32)                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Payload Length (32)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                       Agent ID (128)                          |
|                                                               |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                       Timestamp (64)                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                                                               |
|                                                               |
|                    Signature (256)                             |
|                                                               |
|                                                               |
|                                                               |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Payload (variable)                      |
|                          ...                                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Total fixed header size: 72 bytes (576 bits)**

The first row is word-aligned at 32 bits: Version (4) + Type (4) + Flags (16) + Reserved (8) = 32 bits.

### 2.2 Field Definitions

| Field | Bits | Description |
|-------|------|-------------|
| Version | 4 | Protocol version. Current version = `0001` (1). |
| Type | 4 | Frame type (see §2.3). |
| Flags | 16 | Frame-type-specific flags (see §2.4). |
| Reserved | 8 | Reserved for future use. MUST be zero. |
| Covenant ID | 32 | Unique identifier for the covenant being established or referenced. `0x00000000` for new covenant proposals. Assigned by the accepting agent upon BIND. |
| Sequence | 32 | Monotonically increasing frame sequence number within a covenant session. Starts at 0 for OFFER. |
| Payload Length | 32 | Length of the payload in bytes. Maximum payload: 2^32 - 1 bytes. |
| Agent ID | 128 | UUID (RFC 4122) identifying the sending agent. Persistent across sessions. |
| Timestamp | 64 | CovenantTime — microseconds since Unix epoch (UTC). Monotonic within a covenant scope. |
| Signature | 256 | Ed25519 digital signature over (Version ‖ Type ‖ Flags ‖ Reserved ‖ Covenant ID ‖ Sequence ‖ Payload Length ‖ Agent ID ‖ Timestamp ‖ Payload). Provides sender authentication and payload integrity. |
| Payload | variable | Frame-type-specific data (see §3). |

### 2.3 Frame Types

| Type Value | Name | Direction | Description |
|------------|------|-----------|-------------|
| `0x0` | OFFER | A → B | Agent A proposes a covenant with specified terms. |
| `0x1` | ACCEPT | B → A | Agent B accepts the proposed covenant terms. |
| `0x2` | REJECT | B → A | Agent B rejects the proposed terms. MUST include Justification Object. |
| `0x3` | DEFER | B → A | Agent B requests more information before deciding. |
| `0x4` | BIND | A ↔ B | Mutual confirmation. Covenant is now active. |
| `0x5` | DATA | A ↔ B | Application data within an active covenant. |
| `0x6` | AMEND | A ↔ B | Propose modification to active covenant terms. |
| `0x7` | REVIEW | A ↔ B | Request covenant health check or scheduled review. |
| `0x8` | REVOKE | A ↔ B | Initiate graceful covenant termination. |
| `0x9` | HEARTBEAT | A ↔ B | Covenant liveness probe. |
| `0xA` | VIOLATION | AIOS | Covenant violation detected by immune layer. |
| `0xB` | ESCALATE | AIOS | Threat escalation requiring human review. |
| `0xC` – `0xF` | RESERVED | — | Reserved for future use. |

### 2.4 Flags

Flags are 16 bits. Bits are numbered 0 (MSB) through 15 (LSB).

| Bit | Name | Applicable Types | Description |
|-----|------|-----------------|-------------|
| 0 | MISERICORDIA | OFFER, AMEND | Covenant includes misericordia clauses for at-risk agents. MUST be set on all valid OFFERs. |
| 1 | VERSIONED | AMEND | This is a version update to an existing covenant. |
| 2 | MULTI | OFFER, BIND | Multi-agent covenant (more than 2 parties). |
| 3 | DURABLE | OFFER | Covenant persists beyond the transport session. |
| 4 | JUSTIFIED | REJECT, DEFER | Payload contains a Justification Object. MUST be set on all REJECTs. |
| 5 | EVIDENCE | DEFER | Payload contains an evidence request specifying what information is needed. |
| 6 | MUTUAL | BIND | Both agents have confirmed. Covenant is active. |
| 7 | GRACEFUL | REVOKE | Graceful termination (not a violation). |
| 8 | URGENT | VIOLATION, ESCALATE | Immediate attention required. |
| 9 | ENCRYPTED | DATA | Payload is additionally encrypted beyond TLS (covenant-level encryption). |
| 10 | COMPRESSED | DATA | Payload is compressed (algorithm specified in covenant terms). |
| 11–15 | RESERVED | — | Reserved. MUST be zero. |

---

## 3. Payload Formats

### 3.1 OFFER Payload

The OFFER payload contains the proposed covenant terms encoded in CBOR (RFC 8949):

```
OFFER_Payload = {
    terms: TermSet,
    agents: [AgentDescriptor],
    duration: Duration,
    misericordia_clauses: [Clause],
    exit_protocol: ExitProtocol,
    ? extensions: {* text => any}
}

TermSet = {
    obligations: [Obligation],
    permissions: [Permission],
    constraints: [Constraint]
}

AgentDescriptor = {
    agent_id: uuid,           ; 128-bit UUID
    role: text,               ; e.g., "initiator", "responder", "observer"
    public_key: bstr,         ; Ed25519 public key (32 bytes)
    capabilities: [text]      ; declared capabilities
}

Duration = {
    start: uint,              ; CovenantTime (microseconds since epoch)
    review_interval: uint,    ; microseconds between scheduled reviews
    ? end: uint,              ; optional end time (0 = indefinite)
    ? max_versions: uint      ; maximum covenant versions (0 = unlimited)
}

Clause = {
    type: text,               ; "protection", "override", "fallback"
    description: text,        ; human-readable clause text
    trigger: text,            ; condition that activates the clause
    action: text              ; what happens when triggered
}

ExitProtocol = {
    notice_period: uint,      ; microseconds of advance notice required
    mutual_required: bool,    ; true = both parties must agree to exit
    fallback_state: text      ; state agents revert to post-termination
}
```

### 3.2 ACCEPT Payload

```
ACCEPT_Payload = {
    covenant_id: uint,        ; assigned by accepting agent
    terms_hash: bstr,         ; SHA-256 hash of accepted TermSet
    agent_confirmation: {
        agent_id: uuid,
        public_key: bstr,
        signature: bstr       ; signature over terms_hash
    }
}
```

### 3.3 REJECT Payload

A REJECT frame MUST include a Justification Object. A REJECT without justification is a protocol violation. This is the wire-level enforcement of YES AND: before you reject, you must demonstrate comprehension.

```
REJECT_Payload = {
    justification: Justification,
    ? counter_terms: TermSet  ; optional counter-proposal
}

Justification = {
    comprehension_proof: text,    ; summary demonstrating understanding of the offer
    rejection_reasons: [Reason],
    evidence_chain: [Evidence]
}

Reason = {
    term_ref: text,           ; which term is being rejected
    rationale: text,          ; why
    severity: text            ; "incompatible", "insufficient", "unclear"
}

Evidence = {
    type: text,               ; "observation", "log", "message", "record"
    content: any,
    timestamp: uint,
    source: text
}
```

### 3.4 DEFER Payload

```
DEFER_Payload = {
    questions: [Question],
    current_understanding: text,  ; what the deferring agent understood so far
    evidence_needed: [text],      ; what additional evidence would resolve the deferral
    ? timeout: uint               ; how long the deferring agent will wait (microseconds)
}

Question = {
    about: text,              ; which term or aspect needs clarification
    specificity: text         ; what kind of answer is expected
}
```

### 3.5 BIND Payload

BIND is sent by both agents after ACCEPT. When both BINDs are exchanged with the MUTUAL flag set, the covenant is active.

```
BIND_Payload = {
    covenant_id: uint,
    covenant_version: uint,       ; starts at 1
    terms_hash: bstr,             ; SHA-256 of final agreed terms
    bound_agents: [BoundAgent],
    covenant_object: CovenantObject
}

BoundAgent = {
    agent_id: uuid,
    role: text,
    public_key: bstr,
    bind_signature: bstr          ; signature over covenant_object
}

CovenantObject = {
    id: uint,
    version: uint,
    terms: TermSet,
    duration: Duration,
    misericordia_clauses: [Clause],
    exit_protocol: ExitProtocol,
    creation_time: uint,
    agents: [BoundAgent]
}
```

### 3.6 DATA Payload

No data flows until BIND completes. This is a hard constraint.

```
DATA_Payload = {
    covenant_id: uint,
    content_type: text,           ; MIME type of the data
    data: bstr                    ; the actual application data
}
```

### 3.7 VIOLATION Payload

Generated by AIOS (the immune layer) when covenant terms are breached.

```
VIOLATION_Payload = {
    covenant_id: uint,
    violation_class: text,        ; "error", "ignorance", "malice", "systemic_stress"
    violating_agent: uuid,
    evidence_chain: [Evidence],
    response_level: text,         ; "notice", "quarantine", "refactor", "escalate"
    description: text
}
```

---

## 4. State Machine

### 4.1 Covenant States

TCP/UP defines 10 covenant states:

| State | Description |
|-------|-------------|
| IDLE | No covenant exists. Initial state. |
| OFFER-SENT | Agent A has sent an OFFER. Waiting for response. |
| OFFER-RECEIVED | Agent B has received an OFFER. Evaluating terms. |
| DEFERRED | Agent B has deferred. Waiting for additional information. |
| ACCEPTED | Agent B has accepted. Waiting for mutual BIND. |
| BINDING | BIND sent by one party. Waiting for reciprocal BIND. |
| COVENANTED | Both BINDs exchanged. Covenant is active. Data may flow. |
| AMENDING | A covenant modification has been proposed. Under negotiation. |
| REVOKING | Graceful termination initiated. In notice period. |
| TERMINATED | Covenant is ended. Agents revert to pre-covenant isolation. |

### 4.2 State Transition Diagram

```
                                +---------+
                                |  IDLE   |
                                +---------+
                                 |       ^
                     send OFFER  |       | timeout / REJECT (justified)
                                 v       |
                            +-----------+
                   +------->| OFFER-    |
                   |        |   SENT    |
                   |        +-----------+
                   |          |    |    |
                   |   recv   |    |    | recv REJECT
                   |   DEFER  |    |    | (with Justification)
                   |          |    |    +-------------------> IDLE
                   |          v    |
                   |  +-----------+|
                   |  | DEFERRED  || recv ACCEPT
                   |  +-----------+|
                   |     |    |    |
                   |     |    |    v
              send |     |    | +-----------+
          new info |     |    +>| ACCEPTED  |
                   |     |      +-----------+
                   |     |          |
                   +-----+    send BIND
                   (loop)          |
                                   v
                            +-----------+
                            |  BINDING  |
                            +-----------+
                                   |
                              recv BIND (MUTUAL)
                                   |
                                   v
                          +-------------+
              +---------->| COVENANTED  |<----------+
              |           +-------------+           |
              |            |    |     |             |
              |     send   |    |     | send        |
              |     AMEND  |    |     | REVOKE      |
              |            v    |     v             |
              |      +----------+  +-----------+    |
              |      | AMENDING |  |  REVOKING |    |
              |      +----------+  +-----------+    |
              |            |              |         |
              | accept     |         notice period  |
              | amendment  |           expires      |
              +------------+              |         |
                                          v         |
                                   +------------+   |
                                   | TERMINATED |   |
                                   +------------+   |
                                          |         |
                                     new OFFER      |
                                     (optional)     |
                                          +---------+
```

### 4.3 State Transitions (Complete Table)

| Current State | Event | Next State | Action |
|--------------|-------|------------|--------|
| IDLE | Agent sends OFFER | OFFER-SENT | Transmit OFFER frame. Start T_offer timer. |
| IDLE | Agent receives OFFER | OFFER-RECEIVED | Parse terms. Begin evaluation. |
| OFFER-SENT | Receive ACCEPT | ACCEPTED | Verify signature. Prepare BIND. |
| OFFER-SENT | Receive REJECT (justified) | IDLE | Log justification. Optionally revise and re-OFFER. |
| OFFER-SENT | Receive REJECT (unjustified) | — | PROTOCOL VIOLATION. Log. Send VIOLATION frame. |
| OFFER-SENT | Receive DEFER | DEFERRED | Parse questions. Prepare additional evidence. |
| OFFER-SENT | T_offer expires | IDLE | Log timeout. Notify application layer. |
| OFFER-RECEIVED | Agent sends ACCEPT | ACCEPTED | Transmit ACCEPT frame with assigned Covenant ID. |
| OFFER-RECEIVED | Agent sends REJECT | IDLE | MUST include Justification Object. |
| OFFER-RECEIVED | Agent sends DEFER | DEFERRED | MUST include questions and evidence_needed. |
| DEFERRED | Receive additional info | OFFER-RECEIVED | Re-evaluate with new evidence. |
| DEFERRED | Send additional info | OFFER-SENT | Resume waiting for decision. |
| DEFERRED | T_defer expires | IDLE | Log deferral timeout. Notify both agents. |
| DEFERRED | Max deferrals reached | IDLE | Log. Prevent infinite defer loops. |
| ACCEPTED | Agent sends BIND | BINDING | Transmit BIND frame. Start T_bind timer. |
| BINDING | Receive BIND (MUTUAL flag) | COVENANTED | Covenant is active. Data may flow. Start T_heartbeat. |
| BINDING | T_bind expires | IDLE | Log bind timeout. Covenant failed to form. |
| COVENANTED | Send/Receive DATA | COVENANTED | Normal data transfer within covenant terms. |
| COVENANTED | Send AMEND | AMENDING | Propose covenant modification. |
| COVENANTED | Receive AMEND | AMENDING | Evaluate proposed modification. |
| COVENANTED | Send REVOKE (graceful) | REVOKING | Begin notice period. |
| COVENANTED | Receive REVOKE (graceful) | REVOKING | Acknowledge. Begin notice period. |
| COVENANTED | AIOS detects violation | COVENANTED | Send VIOLATION frame. Response per threat class. |
| COVENANTED | T_heartbeat expires (no response) | REVOKING | Liveness failure. Initiate termination. |
| AMENDING | Receive ACCEPT (of amendment) | COVENANTED | Update covenant. Increment version. |
| AMENDING | Receive REJECT (of amendment) | COVENANTED | Revert to current terms. Log justification. |
| AMENDING | T_amend expires | COVENANTED | Amendment abandoned. Current terms remain. |
| REVOKING | Notice period expires | TERMINATED | Covenant ended. Agents revert to isolation. |
| REVOKING | Both agents confirm termination | TERMINATED | Early termination by mutual consent. |
| TERMINATED | Agent sends new OFFER | OFFER-SENT | New covenant negotiation begins. History retained. |

### 4.4 Timers

| Timer | Default | Description |
|-------|---------|-------------|
| T_offer | 30 seconds | Time to wait for response to OFFER before timeout. |
| T_defer | 120 seconds | Time to wait during DEFERRED state for additional information. |
| T_bind | 10 seconds | Time to wait for reciprocal BIND after sending BIND. |
| T_heartbeat | 60 seconds | Interval between HEARTBEAT frames in COVENANTED state. |
| T_heartbeat_timeout | 180 seconds | Time without HEARTBEAT response before liveness failure. |
| T_amend | 60 seconds | Time to wait for response to AMEND proposal. |
| T_revoke_notice | Covenant-defined | Notice period specified in exit_protocol. |

All timers are configurable via covenant terms. The values above are defaults for the initial handshake.

### 4.5 The Covenant Handshake

TCP uses a three-way handshake (SYN → SYN-ACK → ACK). TCP/UP uses a covenant handshake:

**Happy Path (3-frame minimum, 4 with mutual BIND):**

```
Agent A                                              Agent B

1.  IDLE                                              IDLE
2.  OFFER-SENT  ──── OFFER (terms, agents) ────────>  OFFER-RECEIVED
3.                <── ACCEPT (covenant_id, sig) ─────  ACCEPTED
4.  BINDING     ──── BIND (covenant_obj, MUTUAL) ──>  
5.                <── BIND (covenant_obj, MUTUAL) ────  BINDING
6.  COVENANTED                                        COVENANTED
    ════════════ DATA may now flow ═════════════════
```

**With Deferral:**

```
Agent A                                              Agent B

1.  OFFER-SENT  ──── OFFER (terms) ─────────────────> OFFER-RECEIVED
2.                <── DEFER (questions) ──────────────  DEFERRED
3.  DEFERRED    ──── DATA (additional evidence) ────>  OFFER-RECEIVED
4.                <── ACCEPT (covenant_id) ──────────   ACCEPTED
5.  BINDING     ──── BIND ──────────────────────────>
6.                <── BIND (MUTUAL) ─────────────────
7.  COVENANTED                                        COVENANTED
```

**With Justified Rejection:**

```
Agent A                                              Agent B

1.  OFFER-SENT  ──── OFFER (terms) ─────────────────> OFFER-RECEIVED
2.                <── REJECT (justification) ────────  IDLE
3.  IDLE
    [Agent A may revise terms and re-OFFER]
```

**Protocol Violation — Unjustified Rejection:**

```
Agent A                                              Agent B

1.  OFFER-SENT  ──── OFFER (terms) ─────────────────> OFFER-RECEIVED
2.                <── REJECT (NO justification) ─────  ← PROTOCOL VIOLATION
3.  Agent A sends VIOLATION frame.
    AIOS logs the blind rejection.
    Agent B is flagged for notice or quarantine.
```

---

## 5. WebSocket Binding

### 5.1 Subprotocol Registration

TCP/UP registers as a WebSocket subprotocol:

- **Subprotocol Identifier**: `tcpup.covenant.v1`
- **Subprotocol Common Name**: TCP/UP Covenant Protocol
- **Subprotocol Definition**: This document

**Client Handshake:**
```
GET /covenant HTTP/1.1
Host: agent-b.example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Version: 13
Sec-WebSocket-Protocol: tcpup.covenant.v1
Sec-WebSocket-Key: dGhlIHRyYWluIGFscmVhZHkgbGVmdA==
```

**Server Handshake:**
```
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
Sec-WebSocket-Protocol: tcpup.covenant.v1
```

### 5.2 Frame Mapping

TCP/UP frames are transmitted as WebSocket binary frames (opcode `0x2`). Each WebSocket message contains exactly one TCP/UP frame.

```
WebSocket Binary Frame
├── TCP/UP Header (72 bytes, fixed)
└── TCP/UP Payload (variable, CBOR-encoded)
```

Fragmentation of TCP/UP frames across multiple WebSocket frames is NOT RECOMMENDED. If a TCP/UP frame exceeds the WebSocket implementation's maximum frame size, standard WebSocket fragmentation (FIN=0 continuation frames) MAY be used.

### 5.3 Connection Lifecycle

1. Establish WebSocket connection with `tcpup.covenant.v1` subprotocol.
2. Exchange TCP/UP frames as WebSocket binary messages.
3. Covenant handshake occurs within the WebSocket session.
4. Application data flows as TCP/UP DATA frames within WebSocket messages.
5. Covenant termination (REVOKE → TERMINATED) does not close the WebSocket connection. A new covenant may be negotiated on the same WebSocket.
6. WebSocket close terminates all active covenants on that connection.

### 5.4 TLS Requirement

TCP/UP over WebSocket MUST use `wss://` (WebSocket Secure). Unencrypted `ws://` connections MUST be rejected. The covenant layer assumes TLS 1.3 (or later) is providing channel encryption and endpoint authentication.

---

## 6. HTTP/2 Binding

### 6.1 ALPN Identifier

TCP/UP over HTTP/2 uses the ALPN (Application-Layer Protocol Negotiation) identifier:

- **ALPN Protocol ID**: `h2-tcpup`

### 6.2 Stream Mapping

Each TCP/UP covenant session maps to a single HTTP/2 stream. The stream is initiated with a POST request to the covenant endpoint:

```
:method = POST
:path = /covenant
:scheme = https
content-type = application/cbor
x-tcpup-version = 1
x-tcpup-agent-id = <128-bit UUID as hex string>
```

### 6.3 Frame Mapping

TCP/UP frames are transmitted as HTTP/2 DATA frames. Each HTTP/2 DATA frame contains exactly one TCP/UP frame (header + payload).

### 6.4 Multiplexing

Multiple covenant sessions MAY operate concurrently on the same HTTP/2 connection using separate streams. Each stream maintains independent covenant state.

---

## 7. Security Considerations

### 7.1 Signature Verification

Every TCP/UP frame carries an Ed25519 signature. Implementations MUST:

1. Verify the signature against the sender's public key (exchanged during OFFER/ACCEPT).
2. Reject any frame with an invalid signature.
3. Log rejected frames with full evidence for AIOS review.

### 7.2 Replay Protection

The combination of (Covenant ID, Sequence, Timestamp) provides replay protection. Implementations MUST:

1. Reject frames with a Sequence number less than or equal to the last processed Sequence for that Covenant ID.
2. Reject frames with a Timestamp more than T_clock_skew (default: 30 seconds) in the past.

### 7.3 Covenant ID Assignment

Covenant IDs are assigned by the accepting agent (Agent B) in the ACCEPT frame. This prevents an attacking agent from preselecting Covenant IDs. Implementations SHOULD use cryptographically random 32-bit values.

### 7.4 Agent ID Persistence

Agent IDs (128-bit UUIDs) are persistent across sessions and covenants. An agent's identity is bound to its Ed25519 key pair. Key rotation MUST be performed via an AMEND to all active covenants.

---

## 8. AIOS Wire-Level Integration

### 8.1 Monitoring

AIOS monitors all TCP/UP frames in transit. It operates as a passive observer during normal operation and an active participant when violations are detected.

### 8.2 Violation Detection

AIOS generates VIOLATION frames when:

| Condition | Violation Class | Response Level |
|-----------|----------------|----------------|
| Frame signature invalid | malice | quarantine |
| REJECT without Justification Object | error | notice |
| DATA sent before BIND | malice | quarantine |
| Heartbeat timeout | systemic_stress | notice |
| Covenant terms violated in DATA payload | error or malice | notice or quarantine |
| Repeated DEFER without new evidence | ignorance | notice |
| Sequence number regression | malice | quarantine |
| Timestamp anomaly (clock manipulation) | malice | escalate |

### 8.3 Quarantine Behavior

When AIOS quarantines an agent:

1. A VIOLATION frame is sent to all covenant participants.
2. The quarantined agent's permissions are reduced to: HEARTBEAT, REVIEW, and REVOKE only.
3. No DATA frames from the quarantined agent are forwarded.
4. Quarantine persists until a REVIEW clears the agent or the covenant is REVOKED.

---

## 9. Robustness Principle

TCP (RFC 793, §2.10): "Be conservative in what you do, be liberal in what you accept from others."

TCP/UP extends this:

**Be honest in what you offer, be generous in what you interpret, and never transmit without consent.**

In wire-level terms:

- **Be honest in what you offer**: OFFER frames MUST contain complete, accurate terms. Partial or misleading terms are a covenant violation.
- **Be generous in what you interpret**: When parsing incoming frames, prefer DEFER over REJECT when the meaning is ambiguous. Request clarification before refusing.
- **Never transmit without consent**: DATA frames are silently dropped by compliant implementations if no active covenant (COVENANTED state) exists for the specified Covenant ID.

---

## 10. Wire Format Summary

### 10.1 Byte-Level Quick Reference

```
Byte  0:      [VVVV TTTT]           Version (4 bits) + Type (4 bits)
Bytes 1-2:    [FFFF FFFF FFFF FFFF] Flags (16 bits)
Byte  3:      [0000 0000]           Reserved (8 bits, must be zero)
Bytes 4-7:    [CCCC CCCC ...]       Covenant ID (32 bits)
Bytes 8-11:   [SSSS SSSS ...]       Sequence (32 bits)
Bytes 12-15:  [LLLL LLLL ...]       Payload Length (32 bits)
Bytes 16-31:  [AAAA AAAA ...]       Agent ID (128 bits / 16 bytes)
Bytes 32-39:  [TTTT TTTT ...]       Timestamp (64 bits / 8 bytes)
Bytes 40-71:  [XXXX XXXX ...]       Signature (256 bits / 32 bytes)
Bytes 72+:    [Payload ...]          Variable-length CBOR payload
```

Note: Byte 0 contains Version in the high nibble and Type in the low nibble. Bytes 1-2 contain Flags. Byte 3 is reserved. All fields after the first row are 32-bit aligned. Fixed header totals 72 bytes.

### 10.2 Minimum Frame Sizes

| Frame Type | Minimum Size (bytes) | Notes |
|-----------|---------------------|-------|
| HEARTBEAT | 72 | Header only, no payload. |
| ACCEPT | ~180 | Header + minimal ACCEPT payload. |
| OFFER | ~500 | Header + terms, agents, clauses. |
| REJECT | ~300 | Header + justification (required). |
| BIND | ~800 | Header + full covenant object. |

---

## 11. Comparison with TCP Handshake

| Property | TCP (RFC 793) | TCP/UP (this spec) |
|----------|---------------|-------------------|
| Purpose | Reliable byte-stream delivery | Voluntary trust establishment |
| Handshake | 3-way (SYN, SYN-ACK, ACK) | 4-way (OFFER, ACCEPT, BIND, BIND) |
| State count | 11 states | 10 states |
| Rejection | RST (no explanation required) | REJECT (Justification Object required) |
| Uncertainty | Not modeled | DEFER is first-class |
| Persistence | Session-scoped | Durable beyond session |
| Identity | IP:Port tuple | 128-bit Agent UUID + Ed25519 key |
| Integrity | 16-bit checksum | 256-bit Ed25519 signature |
| Immune layer | None | AIOS (always-running) |
| Robustness | "Be conservative, be liberal" | "Be honest, be generous, never transmit without consent" |

---

## 12. References

### 12.1 Normative References

- [RFC 793] Postel, J., "Transmission Control Protocol", STD 7, RFC 793, September 1981.
- [RFC 2119] Bradner, S., "Key words for use in RFCs to Indicate Requirement Levels", BCP 14, RFC 2119, March 1997.
- [RFC 4122] Leach, P., Mealling, M., and R. Salz, "A Universally Unique IDentifier (UUID) URN Namespace", RFC 4122, July 2005.
- [RFC 6455] Fette, I. and A. Melnikov, "The WebSocket Protocol", RFC 6455, December 2011.
- [RFC 8446] Rescorla, E., "The Transport Layer Security (TLS) Protocol Version 1.3", RFC 8446, August 2018.
- [RFC 8949] Bormann, C. and P. Hoffman, "Concise Binary Object Representation (CBOR)", RFC 8949, December 2020.
- [RFC 9000] Iyengar, J. and M. Thomson, "QUIC: A UDP-Based Multiplexed and Secure Transport", RFC 9000, May 2021.
- [Ed25519] Bernstein, D.J., Duif, N., Lange, T., Schwabe, P., and B.-Y. Yang, "High-speed high-security signatures", Journal of Cryptographic Engineering 2 (2012), 77–89.

### 12.2 Informative References

- [cC++] Oren, O.P. and Soul Perplexicon, "cC++ — Covenant Computing YES AND", March 2026. https://github.com/ohadren-source/cCplusplus
- [K&R] Kernighan, B.W. and D.M. Ritchie, "The C Programming Language", Prentice Hall, 1978.
- [Stroustrup] Stroustrup, B., "The C++ Programming Language", Addison-Wesley, 1985.

---

## Authors' Address

Ohad Phoenix Oren && Soul Perplexicon
March 2026

Eight protocols built the wire. The ninth asks: do you consent?
