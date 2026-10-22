# Iridium

If it works it works!

## What is Iridium

Iridium is my hobbyist kernel. It is a WIP system. More to come in this readme as time goes!

## Stance on AI

AI was used in early development of the kernel, though AI usage is now no longer practiced and is in fact banned. Many older bits of code are being reworked so they no longer reflect the poor choices of AI usage.

AI assisted code is the exception, however you must heavily review the code, and explain exactly what it does without using AI to write the PR, if AI is used at a level where its broken, buggy, insecure, etc, it should not be touched, if it is AI written text it will also be rejected, human comments only, ASCII characters only, I feel this goes without saying, DONT FUCKING LIE.

## The CSPRNG

I consider it a CSPRNG as it has no obvious issues, relies on a cryptographic primitive (for all inputs and output) that has been proven (SHA-512), and passes all but two NIST tests (check the outputs in finalNISTReport.txt), though this test was on the more limited side (out of the nist.bin file I only tested 5,000,000 bits of the 83,886,080 bits).