# Guidelines for AI Usage in the ScummVM Project

This document provides guidance for AI tools and developers using AI
assistance when contributing to the ScummVM project.

## General Guidelines

 - ALL code, including AI-generated or assisted code, MUST comply with our code formatting conventions, code naming conventions, and commit guidelines.

 - Know your personal limits.
   AI assistance is a tool helping with development, not a replacement for development skills.

 - Only use AI assistance in parts of the codebase you are familiar with.
   You MUST test your changes to the best of your ability BEFORE submitting your code.

 - No slop. Do not submit code changes that solely rely on "vibe coding" without
   understanding the generated code.

 - AI must be restrained to write the appropriate amount of code comments. Lengthy LLM-oriented explanations are forbidden.

 - AI assistance MUST be disclosed at least in the PR description.
   Using AI assistance repeatedly without disclosure or with the goal of appearing
   more capable than you actually are WILL have consequences, up to a permanent ban on the ScummVM project resources.

 - Use of AI-assisted tooling to write extensive parts of code is reserved for established team members who are experienced with the codebase and its conventions.

 - Every commit and PR must be done by humans and with proper scoping. All messages must be written by a human being.

 - Code MUST NOT be authored by an AI agent. Even with using AI assistance, you are the sole author of the code you submit. AI agents must never have (Co-) authorship of your code.

## Licensing and Legal Requirements

All contributions must comply with our licensing requirements.
This means that all code must be compatible with GPLv3+.

## Developer's Responsibilities

AI agents cannot be used as an excuse for low quality. The human submitter
is responsible for:

- Reviewing all AI-generated code
- Ensuring compliance with licensing requirements
- Taking full responsibility for the contribution

## Submitting Whole Engines

We, as a project are mostly interested in game preservation and portability. Thus, an engine supporting 
a particular game or games, is not adding value by itself.

The expectation is that when you submit an engine:

- You join the ScummVM Team and become this engine's primary maintainer
- As the primary maintainer:
  - You have a deep understanding of the engine internals
  - Are able to navigate and maintain the engine without AI assistance
  - Are committed to monitoring, engaging with and fixing bug reports filed against the engine

In essence, if you used AI in your engine development you used it as a tool to amplify your own abilities as a developer, not to replace your reasoning and thinking.

Engines fully reversed by a swarm of agents are of no value to us. Save your tokens and run the game in DOSBox, Wine, or some kind of an emulator or a VM.

## Attribution

When AI tools contribute to the project’s development, we require disclosure. In the past, we required attribution in
the commit log messages, but we dropped this as a hard requirement.

## AI and Art

Usage of AI is NOT allowed for creating media assets, such as logos, game icons, etc. We have very skilled artists working on the project. We believe that art is something truly human that must never be replaced by AI.
