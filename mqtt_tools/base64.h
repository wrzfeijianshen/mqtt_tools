#ifndef BASE64_H
#define BASE64_H

#include <QString>

class Base64
{
public:
    /*
     * ���ܣ���̬��Ա���������ֽ�����ת��ΪBase64�����ַ���
     * ����˵����
     *      binaryData��Ҫת�����ֽ�����
     * ����ֵ��
     *      ת����õ���Base64�����ַ���
     * �쳣�׳���
     *      ��
     * ˵����
     *      ��˵���б�ָ���������׳��κ��쳣
     *      ���һ����������û��ָ���쳣˵������ú��������׳��������͵��쳣
     */
    static QString encode(const QByteArray & binaryData);

    /*
     * ���ܣ���̬��Ա��������Base64�����ַ�������Ϊ�ֽ�����
     * ����˵����
     *      base64String��Ҫת����Base64�����ַ���
     * ����ֵ��
     *      �����õ����ֽ�����
     * �쳣�׳���
     *      �׳������쳣
     *          -1�����ݴ���
     * ˵����
     *      �ַ�������������Ŀհ��ַ����س����з������ַ�
     */
    static QByteArray decode(const QString & base64String);
};

#endif // BASE64_H
