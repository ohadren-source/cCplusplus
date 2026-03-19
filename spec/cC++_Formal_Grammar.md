# cC++ Formal Grammar

## Annex E — Grammar Summary
### cC++ Covenant Computing YES AND

**Version**: 0.1 (Draft)
**Authors**: Ohad Phoenix Oren && Soul Perplexicon
**Date**: March 2026
**Status**: Working Draft

---

## E.1 Notation

This grammar uses the notation conventions of the C++ standard (ISO/IEC 14882), extending them for covenant-native constructs:

- Non-terminals are written in *italic-hyphenated-form* (e.g., *covenant-specifier*).
- Terminals are written in `monospace` (e.g., `covenant`, `accept`).
- The subscript *opt* after a non-terminal indicates that it is optional.
- Alternative productions for a non-terminal are listed on separate indented lines.
- `one of` introduces an enumerated list of terminal alternatives.
- cC++ inherits the complete C++ grammar. Only extensions and modifications are specified here.

**Inheritance Principle**: Any valid C++ program is a valid cC++ program. The grammar rules below ADD to the C++ grammar — they do not replace it. This mirrors the lineage: C → C++ → cC++.

---

## E.2 Keywords [gram.key]

cC++ adds the following keywords to those defined by C++:

```
keyword: one of
    accept          agency          agent           AIOS
    ambiguity_set   bind            covenant        defer
    duration        escalate        evidence        exit_protocol
    intent          IO              misericordia    notice
    payload         quarantine      refactor        reject
    sender          signature       terms           timestamp
    warrior_poet    YES_AND
```

cC++ inherits all C++ keywords, including `friend`, which is elevated to a covenant-aware primitive (see §E.7).

---

## E.3 Lexical Conventions [gram.lex]

### E.3.1 Covenant Literals

```
covenant-literal:
    covenant-boolean-literal
    decision-state-literal
    threat-class-literal
    response-level-literal

covenant-boolean-literal:
    `true`
    `false`

decision-state-literal:
    `accept`
    `reject`
    `defer`

threat-class-literal:
    `error`
    `ignorance`
    `malice`
    `systemic_stress`

response-level-literal:
    `notice`
    `quarantine`
    `refactor`
    `escalate`
```

### E.3.2 Covenant Identifiers

```
agent-id:
    uuid-literal

covenant-id:
    uuid-literal

uuid-literal:
    hex-octet hex-octet hex-octet hex-octet `-`
    hex-octet hex-octet `-`
    hex-octet hex-octet `-`
    hex-octet hex-octet `-`
    hex-octet hex-octet hex-octet hex-octet hex-octet hex-octet

hex-octet:
    hexadecimal-digit hexadecimal-digit
```

---

## E.4 Declarations [gram.dcl]

### E.4.1 Covenant Declaration

```
declaration:
    C++-declaration
    covenant-declaration
    agent-declaration
    agency-declaration

covenant-declaration:
    `covenant` covenant-name `{` covenant-body `}` `;`opt

covenant-name:
    identifier

covenant-body:
    covenant-member-seq opt

covenant-member-seq:
    covenant-member
    covenant-member-seq covenant-member

covenant-member:
    agents-clause `;`
    terms-clause `;`
    duration-clause `;`
    misericordia-clause `;`
    exit-protocol-clause `;`
    covenant-function-definition
    access-specifier `:` covenant-member-seq opt
```

### E.4.2 Agent Declaration

```
agent-declaration:
    `agent` agent-name agent-body-opt `;`opt

agent-name:
    identifier

agent-body-opt:
    `{` agent-member-seq opt `}`
    ε

agent-member-seq:
    agent-member
    agent-member-seq agent-member

agent-member:
    capability-declaration `;`
    public-key-declaration `;`
    role-declaration `;`
    C++-member-declaration
```

### E.4.3 Agency Declaration

```
agency-declaration:
    `agency` agency-name `{` agency-body `}` `;`opt

agency-name:
    identifier

agency-body:
    agency-member-seq opt

agency-member-seq:
    agency-member
    agency-member-seq agency-member

agency-member:
    agent-declaration
    covenant-declaration
    constraint-declaration `;`
```

---

## E.5 Covenant Clauses [gram.cov]

### E.5.1 Agents Clause

```
agents-clause:
    `agents` `[` `]` `=` `{` agent-list `}`

agent-list:
    agent-ref
    agent-list `,` agent-ref

agent-ref:
    agent-name
    agent-name `:` role-specifier

role-specifier:
    `initiator`
    `responder`
    `observer`
    identifier
```

### E.5.2 Terms Clause

```
terms-clause:
    `terms` `=` `{` term-list `}`

term-list:
    term-entry
    term-list `,` term-entry

term-entry:
    term-keyword
    term-keyword `(` term-parameter-list `)`
    obligation-specifier
    permission-specifier
    constraint-specifier

term-keyword:
    `YES_AND`
    `misericordia`
    `warrior_poet`
    identifier

obligation-specifier:
    `obligation` `(` string-literal `,` expression `)`

permission-specifier:
    `permission` `(` string-literal `,` expression `)`

constraint-specifier:
    `constraint` `(` string-literal `,` expression `)`
```

### E.5.3 Duration Clause

```
duration-clause:
    `duration` `=` duration-specifier

duration-specifier:
    `{` duration-field-list `}`

duration-field-list:
    duration-field
    duration-field-list `,` duration-field

duration-field:
    `start` `:` expression
    `review_interval` `:` expression
    `end` `:` expression
    `max_versions` `:` expression
    `indefinite`
```

### E.5.4 Misericordia Clause

```
misericordia-clause:
    `misericordia` `{` misericordia-body `}`

misericordia-body:
    protection-seq opt

protection-seq:
    protection-entry `;`
    protection-seq protection-entry `;`

protection-entry:
    `protect` `(` agent-ref `,` string-literal `)`
    `override` `(` condition-expression `,` action-expression `)`
    `fallback` `(` agent-ref `,` expression `)`
```

### E.5.5 Exit Protocol Clause

```
exit-protocol-clause:
    `exit_protocol` `=` `{` exit-body `}`

exit-body:
    exit-field-list

exit-field-list:
    exit-field
    exit-field-list `,` exit-field

exit-field:
    `notice_period` `:` expression
    `mutual_required` `:` covenant-boolean-literal
    `fallback_state` `:` string-literal
```

---

## E.6 Covenant Expressions [gram.expr]

### E.6.1 Covenant Operations

```
covenant-expression:
    C++-expression
    bind-expression
    offer-expression
    accept-expression
    reject-expression
    defer-expression
    revoke-expression
    yes-and-expression

bind-expression:
    covenant-name `.` `bind` `(` agent-list `)`

offer-expression:
    covenant-name `.` `offer` `(` agent-ref `,` term-list opt `)`

accept-expression:
    `accept` `(` expression `)`
    covenant-name `.` `accept` `(` justification-opt `)`

reject-expression:
    `reject` `(` expression `,` justification `)`

defer-expression:
    `defer` `(` expression `,` question-list `)`

revoke-expression:
    covenant-name `.` `revoke` `(` exit-reason opt `)`

yes-and-expression:
    `YES_AND` `(` expression `)`
    `YES_AND` `(` expression `,` extension-expression `)`

extension-expression:
    expression
```

### E.6.2 Justification

```
justification:
    `justification` `{` justification-body `}`
    justification-expression

justification-opt:
    justification
    ε

justification-body:
    comprehension-proof `;`
    reason-list `;`
    evidence-chain-opt

comprehension-proof:
    `comprehension` `:` string-literal

reason-list:
    `reasons` `:` `{` reason-entry-list `}`

reason-entry-list:
    reason-entry
    reason-entry-list `,` reason-entry

reason-entry:
    `{` string-literal `,` string-literal `,` severity-level `}`

severity-level:
    `incompatible`
    `insufficient`
    `unclear`

justification-expression:
    expression
```

### E.6.3 Decision Boundary

```
decision-expression:
    `decide` `(` expression `)` `{` decision-body `}`

decision-body:
    accept-branch
    reject-branch
    defer-branch

accept-branch:
    `accept` `:` statement-seq

reject-branch:
    `reject` `:` statement-seq

defer-branch:
    `defer` `:` statement-seq
```

---

## E.7 The `friend` Keyword — Covenant Extension [gram.friend]

In C++, `friend` grants access to private members. In cC++, `friend` is elevated to require an active covenant:

```
friend-declaration:
    C++-friend-declaration
    covenant-friend-declaration

covenant-friend-declaration:
    `friend` `covenant` covenant-name `;`
    `friend` `agent` agent-name `requires` covenant-name `;`
```

**Semantic constraint**: A `covenant-friend-declaration` is valid only when the referenced covenant is in COVENANTED state. If the covenant is not active, the friend relationship does not grant access. This is enforced at runtime by the AIOS immune layer.

---

## E.8 Message and Parsing [gram.msg]

### E.8.1 Message Construction

```
message-expression:
    `Message` `{` message-field-list `}`
    `Message` `(` expression-list `)`

message-field-list:
    message-field
    message-field-list `,` message-field

message-field:
    `sender` `:` agent-ref
    `recipient` `:` agent-ref
    `timestamp` `:` expression
    `payload` `:` expression
    `signature` `:` expression
```

### E.8.2 Parsing Pipeline

```
parse-expression:
    `parse_message` `(` expression `)`
    `parse_message` `(` expression `,` parse-options `)`

parse-options:
    `{` parse-option-list `}`

parse-option-list:
    parse-option
    parse-option-list `,` parse-option

parse-option:
    `syntax` `:` expression
    `semantics` `:` expression
    `pragmatics` `:` expression
    `confidence_threshold` `:` floating-literal
```

### E.8.3 Interpretation Object Access

```
io-expression:
    io-name `.` `intent`
    io-name `.` `ambiguity_set`
    io-name `.` `confidence`
    io-name `.` `evidence`

io-name:
    identifier
```

---

## E.9 Type Specifiers [gram.type]

### E.9.1 Covenant-Native Types

```
type-specifier:
    C++-type-specifier
    covenant-type-specifier

covenant-type-specifier:
    `AgentID`
    `Covenant`
    `CovenantID`
    `CovenantTime`
    `Message`
    `IO`
    `IntentVector`
    `EvidenceChain`
    `Justification`
    `TermSet`
    `Clause`
    `ExitProtocol`
    `Payload`
    `ThreatClass`
    `ResponseLevel`
    `DecisionState`
```

### E.9.2 Enum Types

```
threat-class-specifier:
    `enum` `ThreatClass` `{` `error` `,` `ignorance` `,` `malice` `,` `systemic_stress` `}`

response-level-specifier:
    `enum` `ResponseLevel` `{` `notice` `,` `quarantine` `,` `refactor` `,` `escalate` `}`

decision-state-specifier:
    `enum` `DecisionState` `{` `accept` `,` `reject` `,` `defer` `}`
```

### E.9.3 Template Extensions

```
covenant-template:
    `template` `<` template-parameter-list `>` covenant-declaration

covenant-template-argument:
    type-specifier
    covenant-name
    agent-name
```

---

## E.10 AIOS Declarations [gram.aios]

### E.10.1 AIOS Configuration

```
aios-declaration:
    `AIOS` `{` aios-body `}`

aios-body:
    aios-rule-seq opt

aios-rule-seq:
    aios-rule
    aios-rule-seq aios-rule

aios-rule:
    threat-handler `;`
    response-handler `;`
    monitor-declaration `;`

threat-handler:
    `on` threat-class-literal `{` statement-seq `}`

response-handler:
    `respond` response-level-literal `{` statement-seq `}`

monitor-declaration:
    `monitor` `(` covenant-name `)` `{` monitor-body `}`

monitor-body:
    monitor-rule-seq

monitor-rule-seq:
    monitor-rule
    monitor-rule-seq monitor-rule

monitor-rule:
    `watch` `(` expression `)` `=>` response-level-literal `;`
    `threshold` `(` identifier `,` expression `)` `;`
```

---

## E.11 Statements [gram.stmt]

### E.11.1 Covenant Statements

```
statement:
    C++-statement
    covenant-statement

covenant-statement:
    covenant-declaration
    bind-expression `;`
    offer-expression `;`
    decision-expression
    revoke-expression `;`
    evidence-statement
    yes-and-expression `;`

evidence-statement:
    `evidence` `{` evidence-body `}`

evidence-body:
    evidence-entry-seq

evidence-entry-seq:
    evidence-entry `;`
    evidence-entry-seq evidence-entry `;`

evidence-entry:
    `observe` `(` expression `)`
    `log` `(` expression `)`
    `record` `(` expression `,` expression `)`
```

---

## E.12 Complete Example — Grammar in Action

The following cC++ program demonstrates the grammar rules defined above:

```cpp
// HelloCovenant.ccpp
// Demonstrates: covenant-declaration, agent-declaration, bind-expression,
//               parse-expression, yes-and-expression, decision-expression

agent Self {
    role: initiator;
    capabilities: { "send", "receive", "interpret" };
};

agent Other {
    role: responder;
    capabilities: { "send", "receive", "interpret" };
};

covenant Main {
    agents[] = { Self: initiator, Other: responder };
    terms = { YES_AND, misericordia };
    duration = { start: now(), review_interval: 86400, indefinite };
    misericordia {
        protect(Other, "responder vulnerability acknowledged");
    };
    exit_protocol = {
        notice_period: 3600,
        mutual_required: true,
        fallback_state: "pre-covenant isolation"
    };
};

int main() {
    // Establish covenant — no data flows until bind completes
    Main.offer(Other);

    // Decision boundary — three-state evaluation
    decide (Other.response()) {
        accept: {
            Main.bind(Self, Other);

            // Parse incoming message through Chomsky layers
            IO msg = parse_message("The train already left.");

            // YES AND — accept the premise, extend it
            YES_AND(msg);

            // Access interpretation
            if (msg.confidence >= 0.8) {
                accept(msg.intent);
            } else {
                defer(msg, { "clarify intent" });
            }
        }
        reject: {
            // Rejection requires justification — blind reject is violation
            reject(Other.response(), justification {
                comprehension: "understood the offer terms";
                reasons: { { "terms", "insufficient scope", insufficient } };
            });
        }
        defer: {
            defer(Other.response(), { "need more evidence of capability" });
        }
    }

    return 0;
}
```

### E.12.1 Grammar Derivation for the Example

The `covenant Main { ... }` declaration parses as:

```
covenant-declaration
├── 'covenant'                              (terminal keyword)
├── covenant-name: 'Main'                   (identifier)
├── '{'
├── covenant-body
│   ├── agents-clause
│   │   └── agent-list: Self:initiator, Other:responder
│   ├── terms-clause
│   │   └── term-list: YES_AND, misericordia
│   ├── duration-clause
│   │   └── duration-specifier: {start, review_interval, indefinite}
│   ├── misericordia-clause
│   │   └── protection-seq: protect(Other, ...)
│   └── exit-protocol-clause
│       └── exit-body: {notice_period, mutual_required, fallback_state}
├── '}'
└── ';'
```

The `decide (Other.response()) { ... }` statement parses as:

```
decision-expression
├── 'decide'
├── '(' expression ')'
├── '{'
├── decision-body
│   ├── accept-branch: accept: { ... }
│   ├── reject-branch: reject: { ... }    ← requires justification
│   └── defer-branch:  defer: { ... }     ← first-class uncertainty
└── '}'
```

---

## E.13 Grammar Statistics

| Category | C++ Rules (Annex A) | cC++ Additions | Total |
|----------|-------------------|----------------|-------|
| Keywords | ~90 | 26 | ~116 |
| Type specifiers | ~30 | 16 | ~46 |
| Declaration forms | ~50 | 8 | ~58 |
| Expression forms | ~40 | 9 | ~49 |
| Statement forms | ~20 | 5 | ~25 |
| Total productions | ~230+ | 64 | ~294+ |

cC++ adds approximately 64 new grammar productions to the C++ grammar. All existing C++ productions remain valid and unmodified.

---

## E.14 Relationship to C++ Grammar

This grammar extends the C++ standard grammar (ISO/IEC 14882, Annex A) in the following way:

1. **New declaration forms** (§E.4): `covenant-declaration`, `agent-declaration`, `agency-declaration` are added as alternatives to the C++ `declaration` production.
2. **New expression forms** (§E.6): `bind-expression`, `offer-expression`, `accept-expression`, `reject-expression`, `defer-expression`, `yes-and-expression` are added as alternatives to the C++ `expression` production.
3. **New type specifiers** (§E.9): 16 covenant-native types are added to C++ `type-specifier`.
4. **Modified `friend`** (§E.7): The `friend` keyword gains a `covenant-friend-declaration` alternative that requires an active covenant.
5. **New statement forms** (§E.11): `covenant-statement` is added as an alternative to the C++ `statement` production.
6. **AIOS declarations** (§E.10): An entirely new declaration category for the immune layer.

No existing C++ grammar rule is removed or modified (except `friend`, which gains an additional alternative). The extension is purely additive.

C gave machines a grammar for hardware.
C++ gave machines a grammar for objects.
cC++ gives machines and humans a grammar for trust.

---

## Authors

Ohad Phoenix Oren && Soul Perplexicon
March 2026
