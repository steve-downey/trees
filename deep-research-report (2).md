# Expanded slide architecture for Object Orientation Reconsidered

## How this maps to your existing org outline

Your current `.org` file already has a strong top-level structure: after the introductory thesis, it moves through historical antecedents, reasons OO won, the gap in theory, the object as identity/behavior/state, common category mistakes, the modern impedance mismatch, and finally a pragmatic contemporary C++ position. It also explicitly frames the object around identity, behavior, and state, and it places special emphasis on Smalltalk’s practical success and C++’s adoption of OO support in a systems language. citeturn0view0turn1view0

The next level that will make the deck feel fully sourced is not to add more top-level sections, but to turn each existing subsection into a short historical-technical arc: one slide for the problem being solved, one for the mechanism introduced, one for what later audiences misread, and one for what remains valuable now. That organization matches both the Smalltalk literature, which repeatedly ties language design to an integrated environment, and Stroustrup’s own explanations of C++ as an attempt to combine Simula-style program organization with C-level systems efficiency. citeturn6view2turn11view2turn4view1turn17view0

## Recommended next-level slide outline

Below, each bold line is a proposed slide title under one of your existing nodes, with a note on the claim to make and the best primary or near-primary citation to anchor in speaker notes.

**Before the Backlash**

- **Simula was about modeled entities that persist through time.** Use this slide to show that the earliest object lineage came from simulation, where a modeled thing needed its own local data and actions. Simula 67 described the object as a self-contained program instance with its own data and behavior; Stroustrup later said that Simula’s class concept let him map application concepts into code directly. citeturn12search21turn4view0

- **Smalltalk made the whole system object-shaped.** The key move was not merely “classes exist,” but that computation was uniformly described as message-sending among communicating objects. Kay’s retrospective calls early Smalltalk the exemplar of a new style of computing, and Ingalls describes Smalltalk’s central metaphor as a universe of communicating objects. citeturn5view0turn6view2

- **Smalltalk’s practical win was the live environment.** This is the slide where you connect the language model to developer experience: fast edit, compile, debug, inspect, browse, and run in one system. Contemporary documentation described Smalltalk-80 as an interactive graphical programming environment with fast response for creating, compiling, debugging, and running code, and Krasner described the system as integrating language, tools, display, and operating-system-like features in a consistent object-oriented way. citeturn11view1turn11view2

- **C++ did not import Smalltalk wholesale; it embedded OO into systems programming.** Stroustrup’s own account is crisp here: he wanted Simula-like program organization together with C-like efficient low-level code, because no existing language did both in 1979. This is the historical justification for a “modern C++ OO” section later in the talk. citeturn4view1turn4view0

**Why OO Won**

- **It raised the abstraction level above C without giving up the machine.** Stroustrup still summarizes C++ as a better C that supports data abstraction, object-oriented programming, and generic programming; the first-edition description likewise stressed type checking, data abstraction, and object-oriented programming while retaining C’s efficiency and notation. citeturn17view0turn4view3

- **It offered enforceable boundaries around state and representation.** This is the strongest “why it worked” slide for both Smalltalk and later OO. Ingalls argued that message-sending decouples intent from implementation and protects internal state; Liskov independently argued that data abstractions hide representations and simplify maintenance because programs can rely on behavior rather than representation. citeturn6view4turn4view7

- **Smalltalk also sold a complete programming experience, not just a type system.** You can make the point that the success people attributed to “OOP” was partly the success of a well-integrated environment: browser, workspace, debugger, inspector, file system interface, and uniform UI conventions. That is one reason later OO languages often inherited the rhetoric of Smalltalk without inheriting the same environment model. citeturn11view1turn11view2

- **Use the CASE/UML slide as a rhetoric slide, not a proof slide.** Your outline already points to “CASE, UML, and the promise of rigor.” I would make this a short historical transition slide: OO was increasingly marketed as if it were the privileged route to disciplined analysis and design, but that claim outran the tighter technical core of objects themselves. This slide is best used to bridge from adoption to theory rather than to carry new technical definitions. citeturn0view0

**The Quiet Gap in the Theory**

- **Frame the gap as a mismatch between practice and what theory centered.** Liskov’s 1987 address is especially useful because it says, plainly, that object-oriented programming is primarily a data abstraction technique, and that inheritance is the additional ingredient. That already suggests that the cleanest formal handle available at the time was abstraction, not a rich account of identity and ongoing mutable state. citeturn4view7

- **Make inheritance look secondary, not foundational.** Liskov concludes that data abstraction is the more important idea, while hierarchy extends its usefulness only in some situations. That gives you a strong primary citation for the claim that OO’s theoretical story was often built around abstraction first and inheritance second. citeturn4view7

- **Then show that later formal work had to reconstruct objects more explicitly.** By the mid-1990s, Abadi and Cardelli’s *A Theory of Objects* presented object calculi meant to explain object semantics and typing rules, including dynamic dispatch, inheritance, and subtyping. That supports a careful claim: object theory exists, but it arrived later and often by formal reconstruction rather than by a single early canonical model comparable to the lambda calculus for FP. citeturn14search4turn14search15

- **Use Smalltalk’s success itself as part of the explanation.** Xerox was already releasing the Smalltalk-80 system beyond PARC, publishing books, distributing the virtual image, and coordinating implementations at Apple, DEC, HP, and Tektronix. In other words, there was strong practical momentum even while the conceptual vocabulary remained partly historical and design-oriented. citeturn11view2turn10search7

**What an Object Actually Is**

- **Identity distinct from value.** This slide should define the talk’s center of gravity. Simula’s object is a self-contained instance; Smalltalk’s objects have their own memory and communicate by messages. That is the historical basis for saying an object is not just a value with methods attached, but an entity whose continuity matters. citeturn12search21turn5view3

- **State as continuity over time.** The Smalltalk principles are useful here: objects have their own memory, and the environment is built around ongoing processes, inspectors, debuggers, and mutable system objects. This gives you concrete evidence that object state was not an incidental implementation detail but central to the model. citeturn5view3turn11view1

- **Behavior as authority over mutation.** Ingalls’ “Messages” and “Modularity” passages are perfect here. The receiver knows how to perform the operation, and no component should depend on the internal details of another. That is the cleanest historical expression of your “behavior as authority over mutation” framing. citeturn6view2turn6view4

**What We Mistook for OO**

- **Encapsulation was never uniquely OO.** Liskov gives you the cleanest citation: data abstraction is the more important idea, and OO is primarily a data abstraction technique plus inheritance. Stroustrup also separates object-oriented programming from the broader and more basic notion of data abstraction. citeturn4view7turn4view4

- **Inheritance became too central because it was the visible differentiator.** Stroustrup’s 1991 essay is especially useful because he explicitly says that the basic support needed for OOP is a class mechanism with inheritance and a dispatch mechanism depending on dynamic type, while concluding that OOP is programming using inheritance and data abstraction is programming using user-defined types. That lets you argue that inheritance became the marker of “OO” even when many practical benefits came from abstraction more generally. citeturn4view4

- **Polymorphism does not require “objects all the way down.”** Your C++ bridge slide belongs here. Stroustrup’s own top-level language description treats generic programming as coequal with object-oriented programming, and his later technical writing emphasizes that C++ often uses parameterization where other languages would use a common base class. That is exactly the opening you need for “polymorphism without objects.” citeturn17view0turn15search19

**The Modern Impedance Mismatch**

- **Value-oriented design vs identity-oriented design.** Put modern C++ in explicit tension with classic object systems: contemporary C++ has strong support for lightweight abstractions and generic programming, while also supporting OOP. That means the language itself invites a deliberate choice between values and identities rather than assuming everything must be heap-allocated object identity. citeturn17view0turn15search14

- **Why Java-style OO can feel wrong in C++.** Stroustrup says this directly in the FAQ: writing Java-style code in C++ can be frustrating and suboptimal, just as writing C-style code in C++ can be. That gives you a source-backed transition from “OO as one useful technique” to “OO used everywhere is a mismatch.” citeturn17view2

- **State is still where the hard cases live.** The Smalltalk side shows why: the live environment centered processes, inspectors, debuggers, files, windows, and other long-lived mutable objects. The theory side shows why the discomfort persists: later object calculi had to devote substantial effort to self, dynamic dispatch, and imperative semantics. citeturn11view1turn14search4

**A Deliberate Use of OO in Contemporary C++**

- **Constrain identity.** Recommend using OO in C++ when you truly need stable identity, lifetime, authority, or dynamic replacement semantics. Stroustrup’s own modern position is that C++ is multi-paradigm and should not force a single style, while his more technical writing rejects a universal `Object` base as an unnecessary, implementation-oriented artifact. citeturn17view2turn15search19

- **Isolate state.** This slide should connect old OO wisdom to modern C++ discipline: localize mutable state behind interfaces that express behavior, not representation. That is directly in the spirit of both Ingalls’ modularity principle and Liskov’s account of data abstraction. citeturn6view4turn4view7

- **Let value types dominate by default.** This is probably your strongest concluding prescription. Modern C++ explicitly supports data abstraction, OOP, and generic programming together, and Stroustrup describes it as a language for lightweight abstractions. That supports a conclusion of “values first, identity where necessary,” rather than “everything is an object because OO.” citeturn17view0turn15search14

## Best original sources to cite in speaker notes

For the **Smalltalk success** thread, the most useful source sequence is: Kay for the historical ambition and “exemplar” claim; Ingalls for the language principles and message-sending model; Krasner and later Smalltalk documentation for the integrated environment; and the release-process material for evidence that Smalltalk-80 was taken seriously enough to be published, distributed, and reimplemented by major vendors. Those sources let you argue that Smalltalk succeeded not just because it had classes, but because it aligned language, UI, tools, and programming process. citeturn5view0turn6view2turn11view2turn11view1turn10search7

For the **OO in C++** thread, the key primary line is Stroustrup himself: Simula provided the organizing insight, C provided the execution model and efficiency constraints, and C++ was designed to support data abstraction, OOP, and later generic programming in one language. His FAQ is also unusually useful for your talk because it explicitly says C++ and Smalltalk are siblings via Simula rather than parent/child, and because it warns against writing Java-style OO in C++. citeturn4view1turn4view0turn17view0turn17view2

For the **“what we mistook for OO”** thread, Liskov is the most valuable source in the whole stack. Her 1987 paper lets you say, with authority, that data abstraction does much of the heavy lifting people later attributed to OO in general, and that inheritance is an extension rather than the whole story. If you want one citation that destabilizes the usual “encapsulation, abstraction, inheritance, polymorphism” catechism, this is it. citeturn4view7

## What I would emphasize in the deck’s argumentative arc

If you want the deck to land as a pragmatic reinterpretation rather than an anti-OO or pro-OO polemic, the strongest line is this: Smalltalk proved that an object/message/state model could be enormously productive when embedded in a live, consistent environment; C++ then proved that some of those ideas could be imported into systems programming without abandoning efficiency; but the industry often conflated those practical wins with a much broader ideology in which every abstraction problem had to be solved by “OO.” The historical sources support that more nuanced story very well. citeturn11view2turn11view1turn17view0turn17view2turn4view7

That leads naturally to the closing claim you seem to want: object orientation is most defensible when the object is treated as a pragmatic unit of identity, behavior, and state, not as a synonym for all abstraction, all polymorphism, or good design in general. Smalltalk’s own principles around messages, modularity, and object memory fit that reading, and modern C++ makes the same conclusion practical by letting you combine values, generic code, and selectively used OO rather than choosing a single dogma. citeturn5view3turn6view2turn6view4turn17view0turn15search19

## Open questions and limits

The weakest part of the source stack above is the **CASE/UML** node: your outline is right to include it, but the most natural primary citations there are Booch and early UML texts, and I did not have line-level access to those book scans in a citable form here. I would also do a second pass if you want exact page-level citations from the **Orange Book** and **Blue Book** rather than nearby web-accessible editions, previews, and contemporary manuals. Even so, the historical and argumentative spine of the deck, especially around Smalltalk’s practical success and C++’s selective adoption of OO, is already well supported by the sources above. citeturn0view0turn7search7turn7search17