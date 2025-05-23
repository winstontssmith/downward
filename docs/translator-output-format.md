# Output of the Fast Downward translator

This page describes the output format of the translator component of
Fast Downward.

## Version history

This page describes **version 3** of the output file format. The
following list gives a brief version history:

-   **Version 1** (introduced 2004): the original Fast Downward translator,
    generating the `output.sas` file with information for the preprocessor and
    search code and a `test.groups` file with some additional information for human
    users. Output files from this version can be identified by their lack of either
    a `version` or `metric` section.
-   **Version 2** (introduced 2008): added the `metric` section and action cost
    information and introduced a third output file, `all.groups`, with mutex
    information used by the landmark generation procedures.  Output files from
    this version can be identified by their lack of a `version` and inclusion
    of a `metric` section.
-   **Version 3** (introduced September 2011): integrated the three
    output files into one and added the `version` section.

## Translator file format

The translator file consists of eight sections:

1.  [Version section](#translator_file_format_version_section)
2.  [Metric section](#translator_file_format_metric_section)
3.  [Variables section](#translator_file_format_variables_section)
4.  [Mutex section](#translator_file_format_mutex_section)
5.  [Initial state section](#translator_file_format_initial_state_section)
6.  [Goal section](#translator_file_format_goal_section)
7.  [Operator section](#translator_file_format_operator_section)
8.  [Axiom section](#translator_file_format_axiom_section)


### Translator file format: version section

The version section includes a version number that is used by the search
component to determine if its input has been generated by a compatible
translator version.

It always looks like this for the version of the translator documented
here:

*Sample version section:*

    begin_version
    3
    end_version


### Translator file format: metric section

The metric section indicates whether action costs are used or not. It
begins with the line "begin_metric", followed by either a 0 or 1. 0
indicates that action costs are not used, and all actions are treated as
unit-cost. 1 indicates that action costs are used. The section ends with
the line "end_metric".

*Sample metric section (Gripper domain):*

    begin_metric
    0
    end_metric


### Translator file format: variables section

*Background:* the translation process works by partitioning the fluent
facts of the grounded PDDL task into sets of mutually exclusive
propositions ("mutex groups"). Such a partitioning is always possible
since a decomposition into trivial mutex groups with only one element
always works. However, the translator prefers to use larger mutex
groups, trying to find a cover with few groups. A mutex group consisting
of facts {*p*_1, ..., *p*_*k*} is turned into a finite-domain
variable with domain {0, 1, ..., *k*}, where value *i* < *k* means
that fact *p*_{*i*+1} is true and all others are false, and value *k*
means that all facts are false. (Sometimes this last value is omitted
because the translator detects that at least one fact from the group
must always be true.)

The variables section begins with a line containing a single number, the
number of finite-domain variables in the task. Following that line, each
variable is defined in sequence.

An variable definition is structured as follows:

-   The first line is "begin_variable".
-   The second line contains the name of the variable (which is
    usually a nondescriptive name like "var7").
-   The third line specifies the axiom layer of the variable.
-   The fourth line specifies the variable's `range`, i.e., the number of
    different values it can take it on. The value of a variable is always
    from the set {0, 1, 2, ..., `range` - 1}.
-   The following `range` lines specify the symbolic names for each of the
    `range` values the variable can take on, one at a time. These typically
    correspond to grounded PDDL facts, except for values that represent
    that none out a set of PDDL facts is true.
-   The final line is "end_variable".

For state variables that do not correspond to axioms, i.e. which are not
computed from the values of other state variables, the axiom layer is
always -1. For state variables that do correspond to axioms, the axiom
layer determines the order of evaluation of axiom rules, described
further below in the section ["Evaluating Axioms"](#evaluating_axioms).

Sample variables section (Gripper domain, `prob01.pddl` from IPC 1998):

      7
      begin_variable
      var0
      -1
      5
      Atom carry(ball1, right)
      Atom carry(ball2, right)
      Atom carry(ball3, right)
      Atom free(right)
      Atom carry(ball4, right)
      end_variable
      begin_variable
      var1
      -1
      5
      Atom carry(ball3, left)
      Atom free(left)
      Atom carry(ball2, left)
      Atom carry(ball1, left)
      Atom carry(ball4, left)
      end_variable
      begin_variable
      var2
      -1
      3
      Atom at(ball4, rooma)
      Atom at(ball4, roomb)
      <none of those>
      end_variable
      begin_variable
      var3
      -1
      3
      Atom at(ball3, rooma)
      Atom at(ball3, roomb)
      <none of those>
      end_variable
      begin_variable
      var4
      -1
      3
      Atom at(ball1, rooma)
      Atom at(ball1, roomb)
      <none of those>
      end_variable
      begin_variable
      var5
      -1
      3
      Atom at(ball2, rooma)
      Atom at(ball2, roomb)
      <none of those>
      end_variable
      begin_variable
      var6
      -1
      2
      Atom at-robby(roomb)
      Atom at-robby(rooma)
      end_variable

The example shows that there are 7 finite-domain variables in this task.
Please note that the order in which the variables are generated and the
order of their values are not deterministic and can vary between
translator runs.

The first variable is not a derived variable (its axiom layer is -1),
and it can take on 5 different values (from the set {0, 1, 2, 3, 4, 5}),
which correspond to the PDDL facts `(carry ball1 right)`, `(carry ball2
right)`, `(carry ball3 right)`, `(free right)`, and `(carry ball4 right)`. This
represents the state of the right gripper. The next variable is similar, but
represents a state of the left gripper.

The third variable is again not derived (axiom layer -1) and takes on
three values, corresponding to ball4 being in rooma, ball4 being in
roomb, and ball4 being in neither room (which implies that it is carried
by either gripper). The next three state variables similarly represent
the other balls, and the final state variable represents the location of
the robot.


### Translator file format: mutex section

The mutex section encodes additional mutual exclusion constraints in the
form of mutex groups (groups of variable/value pairs of which no two can
be simultaneously true).

A mutex group is called *trivial* if it only represents information that
is obvious from the finite-domain representation (that the same variable
cannot hold two different values concurrently). When used with default
options, the translator will discard trivial mutexes, so the search
component can rely on the fact that all mutexes are non-trivial.
However, this is not guaranteed when using translator options
`--translate-options` and `--keep-unreachable-facts`.

The mutex section begins with a line containing a single number, the
number of mutex groups in the task. Following that line, each mutex
group is defined in sequence.

An mutex group definition is structured as follows:

-   The first line is "begin_mutex_group".
-   The second line contains a single number, denoting the number of
    facts in the mutex group.
-   The following lines describe the facts in the group, one line
    for each fact. Each fact is is given by two numbers separated by
    a space, denoting the variable (indexing into the variable
    section above, counting from 0) and a value for that variable
    (indexing into the list of value names for that variable,
    counting from 0).
-   The final line is "end_mutex_group".

*Sample mutex section (Gripper):*

      7
      begin_mutex_group
      4
      1 4
      0 4
      2 0
      2 1
      end_mutex_group
      begin_mutex_group
      4
      1 0
      0 2
      3 0
      3 1
      end_mutex_group
      begin_mutex_group
      4
      1 3
      0 0
      4 0
      4 1
      end_mutex_group
      begin_mutex_group
      5
      1 1
      1 4
      1 0
      1 2
      1 3
      end_mutex_group
      begin_mutex_group
      5
      0 3
      0 4
      0 2
      0 1
      0 0
      end_mutex_group
      begin_mutex_group
      2
      6 1
      6 0
      end_mutex_group
      begin_mutex_group
      4
      1 2
      0 1
      5 0
      5 1
      end_mutex_group

There are 7 mutex groups.

The first group encodes that the following variable/value pairs are
mutually exclusive: variable 1 has value 4; variable 0 has value 4;
variable 2 has value 0; variable 2 has value 1. This corresponds to the
PDDL facts `(carry ball4 left)`, `(carry ball4 right)`, `(at ball4 rooma)`,
`(at ball4 roomb)`.

The second, third and seventh mutex groups encode similar mutual
exclusion constraints for the other three balls.

The fourth, fifth and sixth mutex groups are trivial.


### Translator file format: initial state section

The initial state section begins with the line "begin_state", followed
by one line for each SAS state variable. Each of those lines contains a
single number, denoting the value of the given state variable in the
initial state (for state variables which do not correspond to derived
predicates) or the "default value" of the state variable (for state
variables corresponding to derived predicates; see section ["Evaluating
Axioms"](#evaluating_axioms) below). The section ends with the line "end_state".

Here is the initial state section for the Gripper example:

*Sample initial state section (Gripper domain):*

      begin_state
      3
      1
      0
      0
      0
      0
      1
      end_state

So the initial value of var0 in the example is 3, the initial value of
var1 is 1, the initial values of var2 through var5 are 0, and the
initial value of var6 is 1. Looking up the meaning of these values in
the variable section shown earlier, this means that exactly the
following STRIPS propositions are true in the initial state: `(free right)`,
`(free left)`, `(at ball4 rooma)`, `(at ball3 rooma)`, `(at ball1 rooma)`, `(at
ball2 rooma)`, `(at-robby rooma)`.


### Translator file format: goal section

The goal section begins with the line "begin_goal", followed by a line
which contains the number of goal pairings. This is followed by one line
for each goal pairing, where a goal pairing is given by two numbers
separated by spaces, where the pair "i j" means that
"var`<i>`" must have the value j in the goal. The goal section
ends with the line "end_goal".

Here is the goal section for the Gripper example:

*Sample goal section (Gripper domain):*

      begin_goal
      4
      2 1
      3 1
      4 1
      5 1
      end_goal

We see that there are four goal conditions: Each of the variables var2
through var5 shall assume the value 1. In other words, the goal is
reached if all four balls are in roomb.

Note that the goal condition of the translated task is always a simple
conjunction of atomic goals. If the original PDDL goal is more
complicated, it is transformed by the translator to fit this
requirement. In some cases, this leads to the introduction of derived
variables, even if the original PDDL task did not use derived
predicates.

### Translator file format: operator section

The operator section begins with a line containing a single number, the
number of operators in the task. Following that line, each operator is
defined in sequence.

An operator definition is structured as follows:

-   The first line is "begin_operator".
-   The second line contains the name of the operator.
-   The third line contains a single number, denoting the number of
    prevail conditions.
-   The following lines describe the prevail conditions, one line
    for each condition. A prevail condition is given by two numbers
    separated by spaces, denoting a variable/value pairing in the
    same notation for goals described above.
-   The first line after the prevail conditions contains a single
    number, denoting the number of effects.
-   The following lines describe the effects, one line for each
    effect (read on).
-   The line before last gives the operator cost. This line only
    matters if [metric](#translator_file_format_metric_section) is
    1 (otherwise, any number here will be treated as 1).
-   The final line is "end_operator".

Of these parts, the lines that describe an effect are most complicated
because effects can have associated effect conditions as well as a
condition on the old value of the affected state variable (called a
"precondition" as opposed to a "prevail condition" in the SAS+
literature). An effect is always given in a single line, with the
individual parts separated by spaces. It is structured as follows:

-   First comes the number of effect conditions. In STRIPS domains, this
    will usually be 0 (additional effect conditions can be introduced by
    the translator in rare cases, though).
-   This is followed by one variable/value pair for each effect
    condition. This is given as two numbers like for goal conditions and
    prevail conditions.
-   This is followed by a number denoting the variable affected by the
    effect in the third-last position.
-   This is followed by the value that this variable must have for the
    operator to be applicable (precondition), or -1 if there is no
    particular value that the variable must have. (Note that is truly
    part of the operator precondition and not an effect condition, and
    having it separated from the operator precondition is somewhat
    clumsy. Let's call it a historical accident caused by SAS+'s
    distinction of prevail and preconditions.)
-   Finally, the last number denotes the new value for the affected
    variable.

Even for fairly small examples, the operator section becomes quite big,
so we omit most operator definitions of the Gripper example:

*Sample operator section (Gripper domain):*

      34
      begin_operator
      move rooma roomb
      0
      1
      0 6 1 0
      0
      end_operator
      begin_operator
      pick ball4 rooma left
      1
      6 1
      2
      0 1 1 4
      0 2 0 2
      0
      end_operator
      [... 31 operators omitted]
      begin_operator
      pick ball1 roomb right
      1
      6 0
      2
      0 0 3 0
      0 4 1 2
      0
      end_operator

The example shows that there are 34 operators in this domain, and three
of them are shown in detail.

The first operator is called "move rooma roomb" and has no prevail
conditions (0) and one effect (1). The effect has no associated effect
conditions (0) and affects var6 (6). It requires that the old value of
the variable is 1, so it is only applicable if the robot is in rooma. It
establishes the value 0, so that the robot will be in roomb afterwards.
This domain does not use explicit action_costs (as encoded in the `metric`
section), so the final line is `0`. (The search code will treat problems with
no explicit action costs as unit-cost problems though, so the action will be
handled as if its cost were 1.)

The two pick-up operators are similar, so we only explain the last one.
Its name is "pick ball1 roomb right". It has one prevail condition,
namely that var6 has the value 0 (i.e. the robot is in roomb). It has
two effects. The first effect has no associated conditions, requires
that var0 currently has value 3 (that is, the right gripper is free) and
changes var0 to value 0 (the right gripper carries ball1). The second
effect has no associated conditions either, requires that var4 currently
has value 1 (ball1 is in roomb) and sets it to value 2 (ball1 is in
neither room afterwards). The operator again ends with the line `0` since this
task does not define explicit action costs.

As an example of an operator involving effect conditions and the don't
care value -1 for an effect precondition, consider the following
operator from a task in the Schedule domain:

*Sample operator with effect conditions and cost (Schedule domain,
modified with costs):*

      begin_operator
      do-polish a0
      1
      7 0
      4
      0 24 1 0
      0 3 -1 0
      1 29 1 29 -1 0
      0 22 1 0
      7
      end_operator

The operator is named "do-polish a0". The prevail condition "7 0"
requires that the temperature of object a0 is cold. The four effects of
the operators are:

-   `0 24 1 0`:

    Unconditionally (0), the polisher (var24), which must be non-busy (1), becomes busy (0).`

-   `0 3 -1 0`:

    Unconditionally (0), the surface condition of a0 (var3), whose current status can be anything (-1), will become polished (0).`

-   `1 29 1 29 -1 0`:

    Under the one (1) effect condition that we have not currently scheduled any object (var29 equals 1), we will have scheduled an object afterwards (var29 is set to 0). The variable var29 may have any value currently (-1).`

-   `0 22 1 0`:

    Unconditionally (0), the scheduled-status of a0 (var22), which must be not-scheduled (1), becomes scheduled (0).`

Note that the only effect with an effect condition (1 29 1 29 -1 0)
could be rewritten as (0 29 -1 0) in this situation, because var29 can
only take on the possible values 0 and 1 anyway. However, the translator
does not try to detect and simplify this effect pattern, which occurs
quite commonly in some of the planning benchmarks.

Finally, the 7 before the "end_operator" line indicates that this
operator has a cost of 7.


### Translator file format: axiom section

The axiom section is similar in structure to the operator section, as
axiom rules can be considered to be operators that are automatically
executed whenever applicable. However, the section is somewhat simpler
in structure because axiom rules only ever affect a single state
variable.

Similar to the operator section, the axiom section begins with a line
containing the number of axiom rules. Following that line, each axiom
rule is defined in sequence.

An axiom rule is structured as follows:

-   The first line is "begin_rule"
-   The second line contains a single number, denoting the number of
    conditions in the "body" of the rule.
-   The following lines describe these conditions, one line for each
    condition. A condition is given by two numbers separated by
    spaces, denoting a variable/value pairing. In other words, the
    same notation as for operator prevail conditions is used.
-   The following line contains three numbers, denoting the variable
    affected by the axiom rule, the value that this variable must
    have for this rule to be applicable, and the new value assigned
    to this variable. The variable and this latter value together
    form the "head" of the rule.
-   The final line is "end_rule".

Variables appearing in the head of axiom rules (axiom variables) are
disjoint from variables affected by operators. In the current version of
the translator, axiom variables always have a binary domain, so the
"old value" for the affected variable is always the complement of the
new value and can be safely ignored.

In the Gripper example, the axiom section looks as follows:

*Sample axiom section (Gripper domain):*

      0

This shows that there are no axiom rules in this domain, which is the
case for all pure-STRIPS benchmarks. Axiom rules will of course be
generated for domains that contain derived predicates, but they can also
be generated for PDDL tasks without derived predicates if they use
non-STRIPS features such as universally quantified conditions, as some
of these are compiled away with the help of axioms. As an example, here
is an axiom rules from a Miconic-FullADL task:

*Sample axiom section (Miconic-FullADL domain):*

      1
      begin_rule
      2
      1 0
      3 0
      5 0 1
      end_rule

The axiom section contains a single axiom rule. It has two conditions in
the body, namely that var1 and var3 are both set to 0, i.e. that
passengers p1 and p0 have both been served. The head of the axiom states
that if the condition is satisfied, then var5 will assume the value 1 if
it currently has the value0. Variable var5 corresponds to a proposition
over a newly introduced predicate "new-axiom@9" which has been
generated to simplify the original PDDL goal
`(forall (?p - passenger) (served ?p))`. (Of course, in this case the goal
could also have been transformed to a simple conjunction in a different way
that does not require axioms.)

## Evaluating axioms

State variables that correspond to derived predicates are not directly
affected by operator applications. Instead, their values in a given
world state are computed from the values of the other state variables
using the following algorithm:

-   First, all axiom state variables are set to their default value
    (the one specified in the initial state section).
-   Second, all axiom rules which affect variables at axiom layer 0
    are evaluated. An axiom rule is evaluated by determining whether
    all variable/value pairings in its body match the current state.
    If so, the variable in the head is changed to the value in the
    head. This process is repeated until no further changes occur.
-   Third, all axioms rules which affect variables at axiom layer 1
    are evaluated.
-   Fourth, all axioms rules which affect variables at axiom layer 2
    are evaluated.
-   ...

The semantics of the translation guarantees that the algorithm always
terminates and that the result is independent of the order in which
axiom rules at the same layer are considered.
