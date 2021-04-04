*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    My Test Setup
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Keywords ***
My Test Setup
    Reset Emulation
    Execute Command         path add @${CURDIR}

Feed Constant
    [Arguments]       ${channel}  ${sample}
    Execute Command   sysbus.adc1 FeedSample ${sample} ${channel} -1

Feed Samples
    [Arguments]       ${channel}  ${sample}
    Execute Command   sysbus.adc1 FeedSample ${sample} ${channel} -1

Received Constant
    [Arguments]       ${channel}  ${sample}
    Wait For Line On Uart   ADC Data: ${sample}  timeout=0.2
    Wait For Line On Uart   ADC Data: ${sample}  timeout=0.2
    Wait For Line On Uart   ADC Data: ${sample}  timeout=0.2

Received ADC Data On Channel
    [Arguments]             ${channel}  ${sample}
    Wait For Line On Uart   Channel: ${channel} ADC Data: ${sample}  timeout=0.2

*** Test Cases ***
Polling ADC can read single channel
    Execute Command         include @robot.resc

    Create Terminal Tester  sysbus.uart1

    Start Emulation

    Write Line To Uart  polling-single-conversion\n  waitForEcho=false

    Feed Constant      channel=0  sample=1
    Received Constant  channel=0  sample=1

    Feed Constant      channel=0  sample=5
    Received Constant  channel=0  sample=5

Polling ADC can scan ADC channels
    Execute Command         include @robot.resc

    Create Terminal Tester  sysbus.uart1

    Feed Constant     channel=0   sample=1
    Feed Constant     channel=1   sample=2
    Feed Constant     channel=2   sample=3

    Start Emulation

    Write Line To Uart  polling-scan-regular\n  waitForEcho=false

    Received ADC Data On Channel  channel=0  sample=1
    Received ADC Data On Channel  channel=1  sample=2
    Received ADC Data On Channel  channel=2  sample=3

Interrupt ADC can read single channel
    Execute Command         include @robot.resc

    Create Terminal Tester  sysbus.uart1

    Start Emulation

    Write Line To Uart  interrupt-single-conversion\n  waitForEcho=false

    Feed Constant      channel=0  sample=1
    Received Constant  channel=0  sample=1

    Feed Constant      channel=0  sample=5
    Received Constant  channel=0  sample=5

DMA can scan ADC channels
    Execute Command         include @robot.resc

    Create Terminal Tester  sysbus.uart1

    Feed Constant  channel=0   sample=1
    Feed Constant  channel=1   sample=2
    Feed Constant  channel=2   sample=3

    Start Emulation

    Write Line To Uart  dma-scan-regular\n  waitForEcho=false

    Wait For Line On Uart   Received All Channels: 1 2 3  timeout=0.2
